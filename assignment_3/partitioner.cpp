#include "partitioner.h"
#include <algorithm>
#include <iostream>
#include <climits>
#include <thread>
#include <atomic>
#include <mutex>

// Build a map from block_id to its community partners
std::map<int, std::vector<int>> build_community_map(const Circuit& circuit) {
    std::map<int, std::vector<int>> community_map;
    
    for (const auto& [blk_i, blk_j] : circuit.community_pairs) {
        community_map[blk_i].push_back(blk_j);
        community_map[blk_j].push_back(blk_i);
    }
    
    return community_map;
}

// Sort blocks by fanout (descending) - high fanout first
std::vector<int> sort_blocks_by_fanout(const Circuit& circuit) {
    std::vector<int> block_order = circuit.block_ids;
    
    std::sort(block_order.begin(), block_order.end(),
        [&circuit](int a, int b) {
            return circuit.block_to_nets.at(a).size() > circuit.block_to_nets.at(b).size();
        });
    
    return block_order;
}

// Compute crossing count for a complete assignment
int compute_crossing_count(const Circuit& circuit,
                           const std::vector<Side>& assignment) {
    int count = 0;
    
    for (int net_id : circuit.net_ids) {
        bool has_left = false;
        bool has_right = false;
        
        for (int blk : circuit.net_to_blocks.at(net_id)) {
            if (assignment[blk] == Side::LEFT) has_left = true;
            if (assignment[blk] == Side::RIGHT) has_right = true;
        }
        
        // Net is cut if it has blocks in both partitions
        if (has_left && has_right) {
            count++;
        }
    }
    
    return count;
}

// Compute community cost for a complete assignment
int compute_community_cost(const Circuit& circuit,
                           const std::vector<Side>& assignment) {
    int cost = 0;
    
    for (const auto& [blk_i, blk_j] : circuit.community_pairs) {
        // Cost +1 if community pair is in different partitions
        if (assignment[blk_i] != assignment[blk_j]) {
            cost++;
        }
    }
    
    return cost;
}

// Compute ADDITIONAL cost when assigning block blk to side
// Only checks nets connected to blk and community pairs involving blk
int compute_additional_cost(const Circuit& circuit,
                            const std::vector<Side>& assignment,
                            const std::map<int, std::vector<int>>& community_map,
                            int blk,
                            Side side) {
    int additional = 0;
    
    // Check nets connected to this block
    // A net becomes cut if it now has blocks on both sides
    Side opposite = (side == Side::LEFT) ? Side::RIGHT : Side::LEFT;
    
    for (int net_id : circuit.block_to_nets.at(blk)) {
        // Check if this net already has a block on the opposite side
        bool has_opposite = false;
        for (int other_blk : circuit.net_to_blocks.at(net_id)) {
            if (assignment[other_blk] == opposite) {
                has_opposite = true;
                break;
            }
        }
        
        // If net has block on opposite side, assigning blk to 'side' cuts it
        // But only count if it wasn't already cut!
        if (has_opposite) {
            // Check if this net was already cut (had blocks on 'side' too)
            bool was_already_cut = false;
            for (int other_blk : circuit.net_to_blocks.at(net_id)) {
                if (other_blk != blk && assignment[other_blk] == side) {
                    was_already_cut = true;
                    break;
                }
            }
            if (!was_already_cut) {
                additional++;  // This assignment newly cuts this net
            }
        }
    }
    
    // Check community pairs involving this block
    auto it = community_map.find(blk);
    if (it != community_map.end()) {
        for (int partner : it->second) {
            // If partner is assigned to opposite side, this pair is split
            if (assignment[partner] == opposite) {
                additional++;
            }
        }
    }
    
    return additional;
}

// Compute predicted cuts based on balance constraints
// For nets that have blocks on only ONE side + unassigned blocks:
// If there's not enough room on that side for all unassigned, net WILL be cut
// Also predicts community splits based on same logic
int compute_balance_predicted_cuts(const Circuit& circuit,
                                   const std::vector<Side>& assignment,
                                   int left_count,
                                   int right_count) {
    int half = circuit.num_blocks / 2;
    int spaces_left = half - left_count;   // remaining spots on LEFT
    int spaces_right = half - right_count; // remaining spots on RIGHT
    
    int predicted_cuts = 0;
    
    // Predict net cuts
    for (int net_id : circuit.net_ids) {
        int on_left = 0;
        int on_right = 0;
        int unassigned = 0;
        
        for (int blk : circuit.net_to_blocks.at(net_id)) {
            if (assignment[blk] == Side::LEFT) on_left++;
            else if (assignment[blk] == Side::RIGHT) on_right++;
            else unassigned++;
        }
        
        // Net already cut - don't double count
        if (on_left > 0 && on_right > 0) continue;
        
        // Net has blocks only on LEFT + some unassigned
        if (on_left > 0 && on_right == 0 && unassigned > 0) {
            // For net to be safe, ALL unassigned must go LEFT
            // If not enough room, at least one must go RIGHT → cut!
            if (unassigned > spaces_left) {
                predicted_cuts++;
            }
        }
        
        // Net has blocks only on RIGHT + some unassigned
        if (on_right > 0 && on_left == 0 && unassigned > 0) {
            // For net to be safe, ALL unassigned must go RIGHT
            // If not enough room, at least one must go LEFT → cut!
            if (unassigned > spaces_right) {
                predicted_cuts++;
            }
        }
    }
    
    // Predict community splits
    for (const auto& [blk_i, blk_j] : circuit.community_pairs) {
        Side side_i = assignment[blk_i];
        Side side_j = assignment[blk_j];
        
        // Already split or both assigned same side - skip
        if (side_i != Side::UNASSIGNED && side_j != Side::UNASSIGNED) {
            continue;  // Already counted in current_lb if split
        }
        
        // One on LEFT, one unassigned
        if (side_i == Side::LEFT && side_j == Side::UNASSIGNED) {
            // If no room on LEFT, partner MUST go RIGHT → split!
            if (spaces_left == 0) {
                predicted_cuts++;
            }
        }
        if (side_j == Side::LEFT && side_i == Side::UNASSIGNED) {
            if (spaces_left == 0) {
                predicted_cuts++;
            }
        }
        
        // One on RIGHT, one unassigned
        if (side_i == Side::RIGHT && side_j == Side::UNASSIGNED) {
            // If no room on RIGHT, partner MUST go LEFT → split!
            if (spaces_right == 0) {
                predicted_cuts++;
            }
        }
        if (side_j == Side::RIGHT && side_i == Side::UNASSIGNED) {
            if (spaces_right == 0) {
                predicted_cuts++;
            }
        }
    }
    
    return predicted_cuts;
}

// Compute initial solution: community-aware greedy
// Try to keep community pairs together for better starting point
int compute_initial_solution(const Circuit& circuit,
                             const std::vector<int>& block_order,
                             PartitionResult& result) {
    int half = circuit.num_blocks / 2;
    
    // Create assignment array
    int max_block_id = *std::max_element(circuit.block_ids.begin(), 
                                          circuit.block_ids.end());
    std::vector<Side> assignment(max_block_id + 1, Side::UNASSIGNED);
    
    int left_count = 0;
    int right_count = 0;
    
    // Step 1: Assign community pairs together
    // Alternate sides to keep balance
    bool next_side_left = true;
    
    for (const auto& [blk_i, blk_j] : circuit.community_pairs) {
        // Skip if either block already assigned
        if (assignment[blk_i] != Side::UNASSIGNED || 
            assignment[blk_j] != Side::UNASSIGNED) {
            continue;
        }
        
        // Check if we can fit both on the preferred side
        if (next_side_left && left_count + 2 <= half) {
            assignment[blk_i] = Side::LEFT;
            assignment[blk_j] = Side::LEFT;
            left_count += 2;
            next_side_left = false;  // alternate
        } else if (!next_side_left && right_count + 2 <= half) {
            assignment[blk_i] = Side::RIGHT;
            assignment[blk_j] = Side::RIGHT;
            right_count += 2;
            next_side_left = true;  // alternate
        } else if (left_count + 2 <= half) {
            // Fallback to whichever side has room
            assignment[blk_i] = Side::LEFT;
            assignment[blk_j] = Side::LEFT;
            left_count += 2;
        } else if (right_count + 2 <= half) {
            assignment[blk_i] = Side::RIGHT;
            assignment[blk_j] = Side::RIGHT;
            right_count += 2;
        }
        // If neither side has room for both, leave for later
    }
    
    // Step 2: Assign remaining blocks (follow fanout order)
    for (int blk : block_order) {
        if (assignment[blk] == Side::UNASSIGNED) {
            if (left_count < half) {
                assignment[blk] = Side::LEFT;
                left_count++;
            } else {
                assignment[blk] = Side::RIGHT;
                right_count++;
            }
        }
    }
    
    // Build result sets
    for (int blk : circuit.block_ids) {
        if (assignment[blk] == Side::LEFT) {
            result.left_partition.insert(blk);
        } else {
            result.right_partition.insert(blk);
        }
    }
    
    // Compute cost
    result.crossing_count = compute_crossing_count(circuit, assignment);
    result.community_cost = compute_community_cost(circuit, assignment);
    result.total_cost = result.crossing_count + result.community_cost;
    
    return result.total_cost;
}

// Recursive branch and bound (DFS)
void branch_and_bound(const Circuit& circuit,
                      const std::vector<int>& block_order,
                      const std::map<int, std::vector<int>>& community_map,
                      std::vector<Side>& assignment,
                      int depth,
                      int left_count,
                      int right_count,
                      int current_lb,
                      int& best_cost,
                      PartitionResult& best_result,
                      int& nodes_visited) {
    
    nodes_visited++;
    
    int half = circuit.num_blocks / 2;
    int remaining = circuit.num_blocks - depth;
    
    // Balance pruning: can't exceed half on either side
    if (left_count > half || right_count > half) {
        return;
    }
    
    // Balance pruning: check if we can still achieve balance
    if (left_count + remaining < half || right_count + remaining < half) {
        return;
    }
    
    // Lower bound pruning - now using passed-in current_lb
    if (current_lb >= best_cost) {
        return;
    }
    
    // Tighter LB: compute predicted cuts (full computation - worth it for pruning)
    int predicted_cuts = compute_balance_predicted_cuts(circuit, assignment,
                                                         left_count, right_count);
    if (current_lb + predicted_cuts >= best_cost) {
        return;
    }
    
    // Base case: all blocks assigned (leaf node)
    // At leaf, current_lb IS the actual cost (we computed it incrementally)
    if (depth == circuit.num_blocks) {
        if (current_lb < best_cost) {
            best_cost = current_lb;
            
            // Update best result
            best_result.left_partition.clear();
            best_result.right_partition.clear();
            
            for (int blk : circuit.block_ids) {
                if (assignment[blk] == Side::LEFT) {
                    best_result.left_partition.insert(blk);
                } else {
                    best_result.right_partition.insert(blk);
                }
            }
            
            // Compute actual crossing/community for reporting
            best_result.crossing_count = compute_crossing_count(circuit, assignment);
            best_result.community_cost = compute_community_cost(circuit, assignment);
            best_result.total_cost = current_lb;
            
            std::cout << "  Found better solution: " << current_lb
                      << " (crossing: " << best_result.crossing_count 
                      << ", community: " << best_result.community_cost << ")" << std::endl;
        }
        return;
    }
    
    // Get next block to assign
    int blk = block_order[depth];
    
    // Determine branching order based on community partners
    // Default: try LEFT first
    Side first_side = Side::LEFT;
    Side second_side = Side::RIGHT;
    
    // Check if this block has community partners
    auto it = community_map.find(blk);
    if (it != community_map.end()) {
        // Check if any partner is already assigned
        for (int partner : it->second) {
            if (assignment[partner] == Side::LEFT) {
                // Partner is LEFT, try LEFT first (keep together)
                first_side = Side::LEFT;
                second_side = Side::RIGHT;
                break;
            } else if (assignment[partner] == Side::RIGHT) {
                // Partner is RIGHT, try RIGHT first (keep together)
                first_side = Side::RIGHT;
                second_side = Side::LEFT;
                break;
            }
        }
    }
    
    // Try first side - compute costs incrementally
    assignment[blk] = first_side;
    int additional_first = compute_additional_cost(circuit, assignment, community_map, blk, first_side);
    if (first_side == Side::LEFT) {
        branch_and_bound(circuit, block_order, community_map, assignment, depth + 1,
                         left_count + 1, right_count,
                         current_lb + additional_first,
                         best_cost, best_result, nodes_visited);
    } else {
        branch_and_bound(circuit, block_order, community_map, assignment, depth + 1,
                         left_count, right_count + 1,
                         current_lb + additional_first,
                         best_cost, best_result, nodes_visited);
    }
    
    // Try second side - compute costs incrementally
    assignment[blk] = second_side;
    int additional_second = compute_additional_cost(circuit, assignment, community_map, blk, second_side);
    if (second_side == Side::LEFT) {
        branch_and_bound(circuit, block_order, community_map, assignment, depth + 1,
                         left_count + 1, right_count,
                         current_lb + additional_second,
                         best_cost, best_result, nodes_visited);
    } else {
        branch_and_bound(circuit, block_order, community_map, assignment, depth + 1,
                         left_count, right_count + 1,
                         current_lb + additional_second,
                         best_cost, best_result, nodes_visited);
    }
    
    // Restore (backtrack)
    assignment[blk] = Side::UNASSIGNED;
}

// Parallel version of branch and bound
// Uses atomic best_cost for thread-safe pruning
void branch_and_bound_parallel(const Circuit& circuit,
                               const std::vector<int>& block_order,
                               const std::map<int, std::vector<int>>& community_map,
                               std::vector<Side> assignment,  // COPY - each thread has own copy
                               int depth,
                               int left_count,
                               int right_count,
                               int current_lb,
                               std::atomic<int>& best_cost,
                               PartitionResult& best_result,
                               std::mutex& result_mutex,
                               std::atomic<int>& nodes_visited) {
    
    nodes_visited++;
    
    int half = circuit.num_blocks / 2;
    int remaining = circuit.num_blocks - depth;
    
    // Balance pruning
    if (left_count > half || right_count > half) {
        return;
    }
    if (left_count + remaining < half || right_count + remaining < half) {
        return;
    }
    
    // LB pruning - read atomic best_cost
    if (current_lb >= best_cost.load()) {
        return;
    }
    
    // Tighter LB with prediction
    int predicted_cuts = compute_balance_predicted_cuts(circuit, assignment,
                                                         left_count, right_count);
    if (current_lb + predicted_cuts >= best_cost.load()) {
        return;
    }
    
    // Leaf node
    if (depth == circuit.num_blocks) {
        // Use mutex for updating best_result
        std::lock_guard<std::mutex> lock(result_mutex);
        
        // Double-check inside lock (another thread might have updated)
        if (current_lb < best_cost.load()) {
            best_cost.store(current_lb);
            
            best_result.left_partition.clear();
            best_result.right_partition.clear();
            
            for (int blk : circuit.block_ids) {
                if (assignment[blk] == Side::LEFT) {
                    best_result.left_partition.insert(blk);
                } else {
                    best_result.right_partition.insert(blk);
                }
            }
            
            best_result.crossing_count = compute_crossing_count(circuit, assignment);
            best_result.community_cost = compute_community_cost(circuit, assignment);
            best_result.total_cost = current_lb;
            
            std::cout << "  Found better solution: " << current_lb
                      << " (crossing: " << best_result.crossing_count 
                      << ", community: " << best_result.community_cost << ")" << std::endl;
        }
        return;
    }
    
    // Get next block to assign
    int blk = block_order[depth];
    
    // Determine branching order based on community partners
    Side first_side = Side::LEFT;
    Side second_side = Side::RIGHT;
    
    auto it = community_map.find(blk);
    if (it != community_map.end()) {
        for (int partner : it->second) {
            if (assignment[partner] == Side::LEFT) {
                first_side = Side::LEFT;
                second_side = Side::RIGHT;
                break;
            } else if (assignment[partner] == Side::RIGHT) {
                first_side = Side::RIGHT;
                second_side = Side::LEFT;
                break;
            }
        }
    }
    
    // Try first side
    assignment[blk] = first_side;
    int additional_first = compute_additional_cost(circuit, assignment, community_map, blk, first_side);
    if (first_side == Side::LEFT) {
        branch_and_bound_parallel(circuit, block_order, community_map, assignment, depth + 1,
                                  left_count + 1, right_count,
                                  current_lb + additional_first,
                                  best_cost, best_result, result_mutex, nodes_visited);
    } else {
        branch_and_bound_parallel(circuit, block_order, community_map, assignment, depth + 1,
                                  left_count, right_count + 1,
                                  current_lb + additional_first,
                                  best_cost, best_result, result_mutex, nodes_visited);
    }
    
    // Try second side
    assignment[blk] = second_side;
    int additional_second = compute_additional_cost(circuit, assignment, community_map, blk, second_side);
    if (second_side == Side::LEFT) {
        branch_and_bound_parallel(circuit, block_order, community_map, assignment, depth + 1,
                                  left_count + 1, right_count,
                                  current_lb + additional_second,
                                  best_cost, best_result, result_mutex, nodes_visited);
    } else {
        branch_and_bound_parallel(circuit, block_order, community_map, assignment, depth + 1,
                                  left_count, right_count + 1,
                                  current_lb + additional_second,
                                  best_cost, best_result, result_mutex, nodes_visited);
    }
}

// Main partition function
PartitionResult partition(const Circuit& circuit, int num_threads) {
    std::cout << "\n=== Branch and Bound Partitioner ===" << std::endl;
    std::cout << "Using " << num_threads << " thread(s)" << std::endl;
    
    // Step 1: Sort blocks by fanout (the hint)
    std::vector<int> block_order = sort_blocks_by_fanout(circuit);
    
    std::cout << "Blocks sorted by fanout (top 5):" << std::endl;
    for (int i = 0; i < std::min(5, (int)block_order.size()); i++) {
        int blk = block_order[i];
        std::cout << "  Block " << blk << " (fanout: "
                  << circuit.block_to_nets.at(blk).size() << ")" << std::endl;
    }
    
    // Step 2: Compute initial solution
    PartitionResult best_result;
    int initial_cost = compute_initial_solution(circuit, block_order, best_result);
    
    std::cout << "Initial solution cost: " << initial_cost
              << " (crossing: " << best_result.crossing_count
              << ", community: " << best_result.community_cost << ")" << std::endl;
    
    // Step 3: Set up for branch and bound
    int max_block_id = *std::max_element(circuit.block_ids.begin(),
                                          circuit.block_ids.end());
    std::map<int, std::vector<int>> community_map = build_community_map(circuit);
    
    // Fix first block to LEFT (symmetry breaking)
    int first_blk = block_order[0];
    
    std::cout << "Starting B&B (block " << first_blk << " fixed to LEFT)..." << std::endl;
    
    // Create base assignment with first block fixed to LEFT
    std::vector<Side> base_assignment(max_block_id + 1, Side::UNASSIGNED);
    base_assignment[first_blk] = Side::LEFT;
    
    if (num_threads == 1) {
        // Sequential version
        int best_cost = initial_cost;
        int nodes_visited = 0;
        
        branch_and_bound(circuit, block_order, community_map, base_assignment, 1,
                         1, 0, 0, best_cost, best_result, nodes_visited);
        
        best_result.nodes_visited = nodes_visited;
        
        std::cout << "\n=== Final Result ===" << std::endl;
        std::cout << "Optimal cost: " << best_result.total_cost << std::endl;
        std::cout << "  Crossing count: " << best_result.crossing_count << std::endl;
        std::cout << "  Community cost: " << best_result.community_cost << std::endl;
        std::cout << "Nodes visited: " << nodes_visited << std::endl;
        
    } else {
        // Parallel version - supports any power of 2
        std::atomic<int> best_cost(initial_cost);
        std::atomic<int> nodes_visited(0);
        std::mutex result_mutex;
        
        // Calculate how many blocks to pre-assign: log2(num_threads)
        int depth = 0;
        int temp = num_threads;
        while (temp > 1) {
            depth++;
            temp /= 2;
        }
        // depth = number of blocks to pre-assign (after first block)
        // starting depth for B&B = depth + 1 (since first block is already assigned)
        
        std::vector<std::thread> threads;
        
        // Spawn num_threads threads, each with unique combination
        for (int thread_id = 0; thread_id < num_threads; thread_id++) {
            threads.emplace_back([&, thread_id]() {
                std::vector<Side> assignment = base_assignment;
                int left_count = 1;  // First block is LEFT
                int right_count = 0;
                int current_cost = 0;
                
                // Assign blocks based on bits of thread_id
                for (int bit = 0; bit < depth; bit++) {
                    int blk = block_order[bit + 1];  // +1 because block_order[0] is already assigned
                    Side side = ((thread_id >> bit) & 1) ? Side::RIGHT : Side::LEFT;
                    
                    assignment[blk] = side;
                    current_cost += compute_additional_cost(circuit, assignment, community_map, blk, side);
                    
                    if (side == Side::LEFT) {
                        left_count++;
                    } else {
                        right_count++;
                    }
                }
                
                // Start B&B from depth+1
                branch_and_bound_parallel(circuit, block_order, community_map, assignment, depth + 1,
                                          left_count, right_count, current_cost,
                                          best_cost, best_result, result_mutex, nodes_visited);
            });
        }
        
        // Wait for all threads
        for (auto& t : threads) {
            t.join();
        }
        
        best_result.nodes_visited = nodes_visited.load();
        
        std::cout << "\n=== Final Result ===" << std::endl;
        std::cout << "Optimal cost: " << best_result.total_cost << std::endl;
        std::cout << "  Crossing count: " << best_result.crossing_count << std::endl;
        std::cout << "  Community cost: " << best_result.community_cost << std::endl;
        std::cout << "Nodes visited: " << nodes_visited.load() << std::endl;
    }
    
    return best_result;
}
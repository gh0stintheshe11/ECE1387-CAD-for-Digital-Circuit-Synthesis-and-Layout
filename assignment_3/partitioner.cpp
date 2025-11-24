#include "partitioner.h"
#include <algorithm>
#include <iostream>
#include <climits>

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

// Lower bound function - simple version
// Counts nets already cut + community pairs already split
int lower_bound(const Circuit& circuit,
                const std::vector<Side>& assignment) {
    int lb_crossing = 0;
    int lb_community = 0;
    
    // Count nets that are already cut
    for (int net_id : circuit.net_ids) {
        bool has_left = false;
        bool has_right = false;
        
        for (int blk : circuit.net_to_blocks.at(net_id)) {
            if (assignment[blk] == Side::LEFT) has_left = true;
            if (assignment[blk] == Side::RIGHT) has_right = true;
        }
        
        if (has_left && has_right) {
            lb_crossing++;
        }
    }
    
    // Count community pairs that are already split
    for (const auto& [blk_i, blk_j] : circuit.community_pairs) {
        Side side_i = assignment[blk_i];
        Side side_j = assignment[blk_j];
        
        // Only count if both assigned and in different partitions
        if (side_i != Side::UNASSIGNED && side_j != Side::UNASSIGNED) {
            if (side_i != side_j) {
                lb_community++;
            }
        }
    }
    
    return lb_crossing + lb_community;
}

// Compute initial solution: first half LEFT, second half RIGHT
int compute_initial_solution(const Circuit& circuit,
                             const std::vector<int>& block_order,
                             PartitionResult& result) {
    int half = circuit.num_blocks / 2;
    
    // Create assignment array (index by block_id, need max block id + 1)
    int max_block_id = *std::max_element(circuit.block_ids.begin(), 
                                          circuit.block_ids.end());
    std::vector<Side> assignment(max_block_id + 1, Side::UNASSIGNED);
    
    // Assign first half to LEFT, second half to RIGHT
    for (int i = 0; i < (int)block_order.size(); i++) {
        int blk = block_order[i];
        if (i < half) {
            assignment[blk] = Side::LEFT;
            result.left_partition.insert(blk);
        } else {
            assignment[blk] = Side::RIGHT;
            result.right_partition.insert(blk);
        }
    }
    
    // Compute cost
    result.crossing_count = compute_crossing_count(circuit, assignment);
    result.community_cost = compute_community_cost(circuit, assignment);
    result.total_cost = result.crossing_count + result.community_cost;
    
    return result.total_cost;
}

// Recursive branch and bound
void branch_and_bound(const Circuit& circuit,
                      const std::vector<int>& block_order,
                      const std::map<int, std::vector<int>>& community_map,
                      std::vector<Side>& assignment,
                      int depth,
                      int left_count,
                      int right_count,
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
    
    // Lower bound pruning
    int lb = lower_bound(circuit, assignment);
    if (lb >= best_cost) {
        return;
    }
    
    // Base case: all blocks assigned (leaf node)
    if (depth == circuit.num_blocks) {
        int crossing = compute_crossing_count(circuit, assignment);
        int community = compute_community_cost(circuit, assignment);
        int total = crossing + community;
        
        if (total < best_cost) {
            best_cost = total;
            
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
            
            best_result.crossing_count = crossing;
            best_result.community_cost = community;
            best_result.total_cost = total;
            
            std::cout << "  Found better solution: " << total
                      << " (crossing: " << crossing 
                      << ", community: " << community << ")" << std::endl;
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
    
    // Try first side
    assignment[blk] = first_side;
    if (first_side == Side::LEFT) {
        branch_and_bound(circuit, block_order, community_map, assignment, depth + 1,
                         left_count + 1, right_count,
                         best_cost, best_result, nodes_visited);
    } else {
        branch_and_bound(circuit, block_order, community_map, assignment, depth + 1,
                         left_count, right_count + 1,
                         best_cost, best_result, nodes_visited);
    }
    
    // Try second side
    assignment[blk] = second_side;
    if (second_side == Side::LEFT) {
        branch_and_bound(circuit, block_order, community_map, assignment, depth + 1,
                         left_count + 1, right_count,
                         best_cost, best_result, nodes_visited);
    } else {
        branch_and_bound(circuit, block_order, community_map, assignment, depth + 1,
                         left_count, right_count + 1,
                         best_cost, best_result, nodes_visited);
    }
    
    // Restore (backtrack)
    assignment[blk] = Side::UNASSIGNED;
}

// Main partition function
PartitionResult partition(const Circuit& circuit) {
    std::cout << "\n=== Branch and Bound Partitioner ===" << std::endl;
    
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
    int best_cost = compute_initial_solution(circuit, block_order, best_result);
    
    std::cout << "Initial solution cost: " << best_cost
              << " (crossing: " << best_result.crossing_count
              << ", community: " << best_result.community_cost << ")" << std::endl;
    
    // Step 3: Set up for branch and bound
    int max_block_id = *std::max_element(circuit.block_ids.begin(),
                                          circuit.block_ids.end());
    std::vector<Side> assignment(max_block_id + 1, Side::UNASSIGNED);
    int nodes_visited = 0;
    
    // Build community map for efficient partner lookup
    std::map<int, std::vector<int>> community_map = build_community_map(circuit);
    
    // Fix first block to LEFT (symmetry breaking)
    int first_blk = block_order[0];
    assignment[first_blk] = Side::LEFT;
    
    std::cout << "Starting B&B (block " << first_blk << " fixed to LEFT)..." << std::endl;
    
    // Step 4: Run branch and bound starting from depth 1
    branch_and_bound(circuit, block_order, community_map, assignment, 1,
                     1, 0,  // left_count=1 since first block is LEFT
                     best_cost, best_result, nodes_visited);
    
    // Step 5: Done
    best_result.nodes_visited = nodes_visited;
    
    std::cout << "\n=== Final Result ===" << std::endl;
    std::cout << "Optimal cost: " << best_result.total_cost << std::endl;
    std::cout << "  Crossing count: " << best_result.crossing_count << std::endl;
    std::cout << "  Community cost: " << best_result.community_cost << std::endl;
    std::cout << "Nodes visited: " << nodes_visited << std::endl;
    
    return best_result;
}
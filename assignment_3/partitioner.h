#ifndef PARTITIONER_H
#define PARTITIONER_H

#include "circuit.h"
#include <vector>
#include <map>

// Main function to run branch and bound partitioner
// num_threads: 1 = sequential, 2 or 4 = parallel
PartitionResult partition(const Circuit& circuit, int num_threads = 4);

// Build a map from block_id to its community partners
std::map<int, std::vector<int>> build_community_map(const Circuit& circuit);

// Sort blocks by fanout (descending) - the hint from assignment
std::vector<int> sort_blocks_by_fanout(const Circuit& circuit);

// Compute initial solution (community-aware greedy)
int compute_initial_solution(const Circuit& circuit, 
                             const std::vector<int>& block_order,
                             PartitionResult& result);

// Compute crossing count for a complete assignment
int compute_crossing_count(const Circuit& circuit,
                           const std::vector<Side>& assignment);

// Compute community cost for a complete assignment  
int compute_community_cost(const Circuit& circuit,
                           const std::vector<Side>& assignment);

// Compute ADDITIONAL cost when assigning a specific block to a side
// Only checks nets connected to that block (incremental LB)
int compute_additional_cost(const Circuit& circuit,
                            const std::vector<Side>& assignment,
                            const std::map<int, std::vector<int>>& community_map,
                            int blk,
                            Side side);

// Compute predicted cuts based on balance constraints (full computation)
int compute_balance_predicted_cuts(const Circuit& circuit,
                                   const std::vector<Side>& assignment,
                                   int left_count,
                                   int right_count);

// Recursive branch and bound function (DFS)
// x_position: horizontal position in tree (0.0 to 1.0)
// parent_index: index of parent node in g_tree_nodes (-1 for root)
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
                      int& nodes_visited,
                      double x_position = 0.5,
                      int parent_index = -1);

#endif // PARTITIONER_H
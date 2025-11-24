#ifndef PARTITIONER_H
#define PARTITIONER_H

#include "circuit.h"
#include <vector>
#include <map>

// Main function to run branch and bound partitioner
PartitionResult partition(const Circuit& circuit);

// Build a map from block_id to its community partners
std::map<int, std::vector<int>> build_community_map(const Circuit& circuit);

// Sort blocks by fanout (descending) - the hint from assignment
std::vector<int> sort_blocks_by_fanout(const Circuit& circuit);

// Compute initial solution (first half left, second half right)
int compute_initial_solution(const Circuit& circuit, 
                             const std::vector<int>& block_order,
                             PartitionResult& result);

// Compute crossing count for a complete assignment
int compute_crossing_count(const Circuit& circuit,
                           const std::vector<Side>& assignment);

// Compute community cost for a complete assignment  
int compute_community_cost(const Circuit& circuit,
                           const std::vector<Side>& assignment);

// Lower bound function - returns lower bound on cost for partial assignment
int lower_bound(const Circuit& circuit,
                const std::vector<Side>& assignment);

// Recursive branch and bound function
void branch_and_bound(const Circuit& circuit,
                      const std::vector<int>& block_order,
                      const std::map<int, std::vector<int>>& community_map,
                      std::vector<Side>& assignment,
                      int depth,
                      int left_count,
                      int right_count,
                      int& best_cost,
                      PartitionResult& best_result,
                      int& nodes_visited);

#endif // PARTITIONER_H
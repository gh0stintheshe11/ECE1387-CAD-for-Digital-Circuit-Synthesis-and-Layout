#ifndef CIRCUIT_H
#define CIRCUIT_H

#include <vector>
#include <map>
#include <set>
#include <string>

// Data structure to hold parsed circuit
struct Circuit {
    int num_blocks;
    int num_nets;
    
    // For each block: list of nets it connects to
    std::map<int, std::vector<int>> block_to_nets;
    
    // For each net: list of blocks it connects
    std::map<int, std::vector<int>> net_to_blocks;
    
    // Community pairs: each pair is (block_i, block_j)
    std::vector<std::pair<int, int>> community_pairs;
    
    // All block IDs (in order they appear in file)
    std::vector<int> block_ids;
    
    // All net IDs
    std::set<int> net_ids;
};

// Partition side for each block
enum class Side {
    UNASSIGNED,
    LEFT,
    RIGHT
};

// Result of partitioning
struct PartitionResult {
    std::set<int> left_partition;
    std::set<int> right_partition;
    int crossing_count;
    int community_cost;
    int total_cost;
    int nodes_visited;
};

#endif // CIRCUIT_H
#ifndef GRAPHICS_H
#define GRAPHICS_H

#include <vector>
#include <string>
#include "circuit.h"

// Represents a node in the decision tree for visualization
struct TreeNode {
    int depth;              // Depth in tree (0 = root)
    int block_id;           // Which block was assigned at this node
    Side side;              // LEFT or RIGHT
    bool pruned;            // Was this node pruned?
    bool is_solution;       // Is this a leaf with valid solution?
    double x_position;      // Horizontal position (0.0 to 1.0)
    int parent_index;       // Index of parent node (-1 for root)
};

// Global storage for tree visualization
extern std::vector<TreeNode> g_tree_nodes;
extern int g_max_depth;
extern std::vector<int> g_block_order;
extern bool g_graphics_enabled;

// Record a visited node, returns index
int record_tree_node(int depth, int block_id, Side side, double x_position, int parent_index);

// Mark node as pruned
void mark_node_pruned(int node_index);

// Mark node as solution
void mark_node_solution(int node_index);

// Clear tree data
void clear_tree_nodes();

// Launch EZGL graphics window
void run_graphics(const std::string& circuit_name, int optimal_cost, int nodes_visited);

#endif
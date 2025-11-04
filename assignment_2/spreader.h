#ifndef SPREADER_H
#define SPREADER_H

#include "data_structures.h"
#include <vector>
#include <map>
#include <string>

// Spreading mode
enum class SpreadMode {
    HOMOGENEOUS,   // Any type can go anywhere
    HETEROGENEOUS  // Type 0: cols 0,1,2 (mod 4), Type 1: col 3 (mod 4)
};

// Spreading result statistics
struct SpreadStats {
    double total_displacement;  // Sum of all cell movements
    double hpwl_before;         // HPWL before spreading
    double hpwl_after;          // HPWL after spreading
    int iterations;             // Number of spreading iterations
    double final_psi;           // Final ψ value used
};

// Main spreading function - Part 2i
SpreadStats apply_flow_based_spreading(
    SpreadMode mode,
    double psi_init,      // Initial ψ value
    double psi_incr,      // ψ increment per iteration
    int max_iterations = 100
);

// Anchoring function - Part 2ii
// Creates artificial fixed blocks at current cell positions
// Adds pseudo nets connecting cells to anchors with given weight
void create_anchors_and_pseudo_nets(double anchor_weight);

// Helper function to check if a cell type can be placed in a column
bool is_legal_column(int cell_type, int bin_col);

// Print spreading statistics
void print_spread_stats(const SpreadStats& stats, SpreadMode mode);

#endif // SPREADER_H
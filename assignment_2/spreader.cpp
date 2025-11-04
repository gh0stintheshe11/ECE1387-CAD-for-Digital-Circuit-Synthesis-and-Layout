#include "spreader.h"
#include "data_structures.h"
#include "hpwl_calculator.h"
#include <iostream>
#include <cmath>
#include <algorithm>
#include <queue>
#include <limits>

// Grid parameters
const int GRID_SIZE = 40;  // 40×40 grid
const double BIN_WIDTH = 1.0;
const double BIN_HEIGHT = 1.0;
const double PLACEMENT_MIN = 0.0;
const double PLACEMENT_MAX = 40.0;

// Global bins (40×40 = 1600 bins)
std::vector<std::vector<Bin>> bin_grid;

// Store anchor information for Part 2ii
struct AnchorInfo {
    int cell_id;
    double anchor_x;
    double anchor_y;
};
std::vector<AnchorInfo> anchors;

// Initialize bin grid
void initialize_bins() {
    bin_grid.clear();
    bin_grid.resize(GRID_SIZE);
    
    for (int col = 0; col < GRID_SIZE; col++) {
        bin_grid[col].resize(GRID_SIZE);
        for (int row = 0; row < GRID_SIZE; row++) {
            double x_min = PLACEMENT_MIN + col * BIN_WIDTH;
            double x_max = x_min + BIN_WIDTH;
            double y_min = PLACEMENT_MIN + row * BIN_HEIGHT;
            double y_max = y_min + BIN_HEIGHT;
            
            bin_grid[col][row] = Bin(col, row, x_min, x_max, y_min, y_max);
        }
    }
}

// Get bin indices for a coordinate
void get_bin_indices(double x, double y, int& col, int& row) {
    col = static_cast<int>((x - PLACEMENT_MIN) / BIN_WIDTH);
    row = static_cast<int>((y - PLACEMENT_MIN) / BIN_HEIGHT);
    
    // Clamp to valid range
    col = std::max(0, std::min(GRID_SIZE - 1, col));
    row = std::max(0, std::min(GRID_SIZE - 1, row));
}

// Check if cell type can be placed in column (heterogeneous constraint)
bool is_legal_column(int cell_type, int bin_col) {
    if (cell_type == 0) {
        // Type 0: can be in columns where col % 4 = 0, 1, or 2
        return (bin_col % 4) != 3;
    } else {
        // Type 1: can only be in columns where col % 4 = 3
        return (bin_col % 4) == 3;
    }
}

// Assign cells to bins based on current positions
void assign_cells_to_bins() {
    // Clear all bins
    for (int col = 0; col < GRID_SIZE; col++) {
        for (int row = 0; row < GRID_SIZE; row++) {
            bin_grid[col][row].cells.clear();
        }
    }
    
    // Assign each moveable block to its bin
    for (auto& [id, block] : blocks) {
        if (block.is_fixed) continue;
        
        int col, row;
        get_bin_indices(block.x, block.y, col, row);
        bin_grid[col][row].cells.push_back(id);
    }
}

// Calculate relative position of cell within its bin
void get_relative_position(int block_id, double& rel_x, double& rel_y) {
    const Block& block = blocks.at(block_id);
    int col, row;
    get_bin_indices(block.x, block.y, col, row);
    
    const Bin& bin = bin_grid[col][row];
    
    // Relative position (0 to 1) within bin
    rel_x = (block.x - bin.x_min) / BIN_WIDTH;
    rel_y = (block.y - bin.y_min) / BIN_HEIGHT;
    
    // Clamp to [0, 1]
    rel_x = std::max(0.0, std::min(1.0, rel_x));
    rel_y = std::max(0.0, std::min(1.0, rel_y));
}

// Move cell to new bin, preserving relative position
void move_cell_to_bin(int block_id, int target_col, int target_row) {
    Block& block = blocks.at(block_id);
    
    // Get relative position in current bin
    double rel_x, rel_y;
    get_relative_position(block_id, rel_x, rel_y);
    
    // Calculate new position in target bin
    const Bin& target_bin = bin_grid[target_col][target_row];
    block.x = target_bin.x_min + rel_x * BIN_WIDTH;
    block.y = target_bin.y_min + rel_y * BIN_HEIGHT;
}

// Simple greedy spreading algorithm (one iteration)
// For each overcapacity bin, move excess cells to nearby bins
bool spread_iteration(SpreadMode mode, double psi) {
    bool any_overflow = false;
    int cells_moved = 0;
    
    // Find all overcapacity bins
    std::vector<std::pair<int, int>> overcap_bins;
    for (int col = 0; col < GRID_SIZE; col++) {
        for (int row = 0; row < GRID_SIZE; row++) {
            if (bin_grid[col][row].overflow() > 0.01) {
                overcap_bins.push_back({col, row});
                any_overflow = true;
            }
        }
    }
    
    if (!any_overflow) {
        return false;  // Converged!
    }
    
    // For each overcapacity bin, try to move excess cells
    for (auto [col, row] : overcap_bins) {
        Bin& bin = bin_grid[col][row];
        
        while (bin.overflow() > 0.01 && !bin.cells.empty()) {
            // Find best target bin within psi distance
            int best_target_col = col;
            int best_target_row = row;
            double best_score = 1e9;
            
            int search_radius = static_cast<int>(std::ceil(psi));
            for (int dc = -search_radius; dc <= search_radius; dc++) {
                for (int dr = -search_radius; dr <= search_radius; dr++) {
                    int target_col = col + dc;
                    int target_row = row + dr;
                    
                    // Check bounds
                    if (target_col < 0 || target_col >= GRID_SIZE ||
                        target_row < 0 || target_row >= GRID_SIZE) continue;
                    
                    // Check distance constraint
                    double dist = std::sqrt(dc*dc + dr*dr);
                    if (dist > psi) continue;
                    
                    // Check if target has capacity
                    Bin& target = bin_grid[target_col][target_row];
                    if (target.demand() >= target.capacity) continue;
                    
                    // Check heterogeneous constraint
                    if (mode == SpreadMode::HETEROGENEOUS) {
                        int cell_id = bin.cells.back();
                        int cell_type = blocks.at(cell_id).type;
                        if (!is_legal_column(cell_type, target_col)) continue;
                    }
                    
                    // Score: prefer closer bins with more capacity
                    double score = dist - (target.capacity - target.demand());
                    if (score < best_score) {
                        best_score = score;
                        best_target_col = target_col;
                        best_target_row = target_row;
                    }
                }
            }
            
            // Move cell if we found a valid target
            if (best_target_col != col || best_target_row != row) {
                int cell_id = bin.cells.back();
                bin.cells.pop_back();
                
                move_cell_to_bin(cell_id, best_target_col, best_target_row);
                bin_grid[best_target_col][best_target_row].cells.push_back(cell_id);
                cells_moved++;
            } else {
                // Can't move this cell, try next one or give up
                break;
            }
        }
    }
    
    std::cout << "  Moved " << cells_moved << " cells" << std::endl;
    return true;  // Continue iterations
}

// Calculate total cell displacement from original positions
double calculate_total_displacement(const std::map<int, Block>& original_blocks) {
    double total = 0.0;
    
    for (const auto& [id, block] : blocks) {
        if (block.is_fixed) continue;
        
        const Block& orig = original_blocks.at(id);
        double dx = block.x - orig.x;
        double dy = block.y - orig.y;
        total += std::abs(dx) + std::abs(dy);  // Manhattan distance
    }
    
    return total;
}

// Main spreading algorithm - Part 2i
SpreadStats apply_flow_based_spreading(
    SpreadMode mode,
    double psi_init,
    double psi_incr,
    int max_iterations
) {
    std::cout << "\n========================================" << std::endl;
    std::cout << "    Flow-Based Spreading Algorithm" << std::endl;
    std::cout << "========================================" << std::endl;
    
    std::cout << "\nMode: " << (mode == SpreadMode::HOMOGENEOUS ? "Homogeneous" : "Heterogeneous") << std::endl;
    std::cout << "Initial ψ: " << psi_init << std::endl;
    std::cout << "ψ increment: " << psi_incr << std::endl;
    std::cout << "Max iterations: " << max_iterations << std::endl;
    
    // Save original positions
    std::map<int, Block> original_blocks = blocks;
    
    // Calculate initial HPWL
    double hpwl_before = calculate_hpwl();
    
    // Initialize bin grid
    initialize_bins();
    
    // Assign cells to bins
    assign_cells_to_bins();
    
    // Report initial statistics
    int total_moveable = 0;
    int total_overflow_bins = 0;
    double total_overflow = 0.0;
    
    for (int col = 0; col < GRID_SIZE; col++) {
        for (int row = 0; row < GRID_SIZE; row++) {
            const Bin& bin = bin_grid[col][row];
            total_moveable += bin.cells.size();
            if (bin.overflow() > 0.01) {
                total_overflow_bins++;
                total_overflow += bin.overflow();
            }
        }
    }
    
    std::cout << "\nInitial state:" << std::endl;
    std::cout << "  Total moveable cells: " << total_moveable << std::endl;
    std::cout << "  Total bins: " << (GRID_SIZE * GRID_SIZE) << std::endl;
    std::cout << "  Overcapacity bins: " << total_overflow_bins << std::endl;
    std::cout << "  Total overflow: " << total_overflow << std::endl;
    std::cout << "  Initial HPWL: " << hpwl_before << std::endl;
    
    // Spreading loop
    double psi = psi_init;
    int iteration = 0;
    bool converged = false;
    
    std::cout << "\nSpreading iterations:" << std::endl;
    
    while (iteration < max_iterations && !converged) {
        std::cout << "\nIteration " << (iteration + 1) << " (ψ = " << psi << "):" << std::endl;
        
        // Re-assign cells to bins (they may have moved)
        assign_cells_to_bins();
        
        // Count overflow
        total_overflow_bins = 0;
        for (int col = 0; col < GRID_SIZE; col++) {
            for (int row = 0; row < GRID_SIZE; row++) {
                if (bin_grid[col][row].overflow() > 0.01) {
                    total_overflow_bins++;
                }
            }
        }
        
        std::cout << "  Overcapacity bins: " << total_overflow_bins << std::endl;
        
        if (total_overflow_bins == 0) {
            std::cout << "  ✓ All bins meet capacity constraints!" << std::endl;
            converged = true;
            break;
        }
        
        // Perform spreading iteration
        spread_iteration(mode, psi);
        
        // Increase psi for next iteration
        psi += psi_incr;
        iteration++;
    }
    
    if (!converged) {
        std::cout << "\n[WARNING] Did not fully converge after " << max_iterations << " iterations" << std::endl;
    }
    
    // Calculate final statistics
    double hpwl_after = calculate_hpwl();
    double total_displacement = calculate_total_displacement(original_blocks);
    
    SpreadStats stats;
    stats.total_displacement = total_displacement;
    stats.hpwl_before = hpwl_before;
    stats.hpwl_after = hpwl_after;
    stats.iterations = iteration;
    stats.final_psi = psi - psi_incr;  // Last used psi
    
    return stats;
}

// Part 2ii: Create anchors and pseudo nets
void create_anchors_and_pseudo_nets(double anchor_weight) {
    std::cout << "\n========================================" << std::endl;
    std::cout << "    Creating Anchors & Pseudo Nets" << std::endl;
    std::cout << "========================================" << std::endl;
    
    // Clear previous anchors
    anchors.clear();
    
    // SET GLOBAL ANCHOR WEIGHT
    global_anchor_weight_scale = anchor_weight;  // NEW LINE
    
    // Find the highest existing block ID and net ID
    int max_block_id = 0;
    int max_net_id = 0;
    
    for (const auto& [id, block] : blocks) {
        if (id > max_block_id) max_block_id = id;
    }
    for (const auto& [id, net] : nets) {
        if (id > max_net_id) max_net_id = id;
    }
    
    int anchor_block_id = max_block_id + 1;
    int pseudo_net_id = max_net_id + 1;
    
    std::cout << "\nCreating anchors for moveable cells..." << std::endl;
    std::cout << "Anchor weight: " << anchor_weight << std::endl;
    
    int anchor_count = 0;
    
    // For each moveable cell, create an anchor at its current (spread) position
    for (auto& [cell_id, cell_block] : blocks) {
        if (cell_block.is_fixed) continue;  // Skip already fixed blocks
        
        // Save anchor information
        AnchorInfo anchor_info;
        anchor_info.cell_id = cell_id;
        anchor_info.anchor_x = cell_block.x;  // Current spread position
        anchor_info.anchor_y = cell_block.y;
        anchors.push_back(anchor_info);
        
        // Create artificial fixed block at spread position
        Block anchor_block;
        anchor_block.id = anchor_block_id;
        anchor_block.type = cell_block.type;  // Same type as cell
        anchor_block.x = cell_block.x;        // Spread position
        anchor_block.y = cell_block.y;
        anchor_block.is_fixed = true;         // Anchor is fixed!
        anchor_block.is_anchor = true;        // Mark as anchor
        
        blocks[anchor_block_id] = anchor_block;
        
        // Create pseudo net connecting cell to its anchor
        Net pseudo_net;
        pseudo_net.id = pseudo_net_id;
        pseudo_net.blocks.push_back(cell_id);          // Moveable cell
        pseudo_net.blocks.push_back(anchor_block_id);  // Fixed anchor
        pseudo_net.is_pseudo = true;                   // NEW: Mark as pseudo net
        
        nets[pseudo_net_id] = pseudo_net;
        
        // Add pseudo net to both blocks' net lists
        cell_block.nets.push_back(pseudo_net_id);
        anchor_block.nets.push_back(pseudo_net_id);
        blocks[anchor_block_id] = anchor_block;  // Update anchor with net
        
        anchor_block_id++;
        pseudo_net_id++;
        anchor_count++;
    }
    
    std::cout << "Created " << anchor_count << " anchors" << std::endl;
    std::cout << "Created " << anchor_count << " pseudo nets (2-pin each)" << std::endl;
    std::cout << "Total blocks now: " << blocks.size() << std::endl;
    std::cout << "Total nets now: " << nets.size() << std::endl;
    
    std::cout << "\n[INFO] Anchors created. Weight matrix must be rebuilt." << std::endl;
    std::cout << "[INFO] Pseudo net weights will be scaled by " << anchor_weight << std::endl;
    std::cout << "========================================" << std::endl;
}

void print_spread_stats(const SpreadStats& stats, SpreadMode mode) {
    std::cout << "\n========================================" << std::endl;
    std::cout << "      Spreading Results Summary" << std::endl;
    std::cout << "========================================" << std::endl;
    
    std::cout << "\nMode: " << (mode == SpreadMode::HOMOGENEOUS ? "Homogeneous" : "Heterogeneous") << std::endl;
    
    std::cout << "\nIterations: " << stats.iterations << std::endl;
    std::cout << "Final ψ: " << stats.final_psi << std::endl;
    
    std::cout << "\nHPWL:" << std::endl;
    std::cout << "  Before spreading: " << stats.hpwl_before << std::endl;
    std::cout << "  After spreading:  " << stats.hpwl_after << std::endl;
    std::cout << "  Change: " << (stats.hpwl_after - stats.hpwl_before) 
              << " (" << std::showpos << ((stats.hpwl_after / stats.hpwl_before - 1.0) * 100.0) 
              << std::noshowpos << "%)" << std::endl;
    
    std::cout << "\nTotal Cell Displacement: " << stats.total_displacement << std::endl;
    
    std::cout << "========================================\n" << std::endl;
}
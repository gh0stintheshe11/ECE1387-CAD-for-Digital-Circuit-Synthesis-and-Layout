#include "weight_matrix.h"
#include "data_structures.h"
#include <iostream>
#include <iomanip>
#include <map>
#include <vector>
#include <cmath>

// External global (defined in data_structures.h)
extern double global_anchor_weight_scale;

// Helper: Calculate weight for an edge in clique model
// For a net with p pins, each edge has weight = 2/p
double calculate_clique_weight(int num_pins) {
    if (num_pins < 2) return 0.0;
    return 2.0 / num_pins;
}

bool build_placement_system(
    SparseMatrix& Q,
    std::vector<double>& b_x,
    std::vector<double>& b_y
) {
    std::cout << "\n========================================" << std::endl;
    std::cout << "  Building Placement System (Qx = b)" << std::endl;
    std::cout << "========================================" << std::endl;
    
    // Step 1: Create mapping from block ID to matrix index
    // Only moveable blocks get an index (0 to n-1)
    std::map<int, int> block_to_index;  // blockID -> matrix index
    std::vector<int> index_to_block;    // matrix index -> blockID
    
    int index = 0;
    for (const auto& [block_id, block] : blocks) {
        // NEW: Skip anchors too (treat them like fixed blocks)
        if (!block.is_fixed && !block.is_anchor) {
            block_to_index[block_id] = index;
            index_to_block.push_back(block_id);
            index++;
        }
    }
    
    int n = index;  // Number of moveable blocks
    std::cout << "\nMoveable blocks: " << n << std::endl;
    
    if (n == 0) {
        std::cerr << "Error: No moveable blocks found!" << std::endl;
        return false;
    }
    
    // Initialize b vectors
    b_x.assign(n, 0.0);
    b_y.assign(n, 0.0);
    
    // Step 2: Build Q matrix using COO (Coordinate) format first
    // We'll store (row, col, value) triplets, then convert to compressed column
    std::map<std::pair<int,int>, double> Q_coo;  // (row, col) -> value
    
    std::cout << "\nProcessing nets with clique model..." << std::endl;
    int nets_processed = 0;
    
    // Step 3: Process each net
    for (const auto& [net_id, net] : nets) {
        int p = net.blocks.size();  // Number of pins
        if (p < 2) continue;  // Skip single-pin nets
        
        // NEW: Calculate base weight, then scale for pseudo nets
        double weight = calculate_clique_weight(p);
        if (net.is_pseudo) {
            weight *= global_anchor_weight_scale;
        }
        
        // Separate moveable and fixed blocks in this net
        std::vector<int> moveable_in_net;
        std::vector<int> fixed_in_net;
        
        for (int block_id : net.blocks) {
            // NEW: Treat anchors as fixed
            if (blocks[block_id].is_fixed || blocks[block_id].is_anchor) {
                fixed_in_net.push_back(block_id);
            } else {
                moveable_in_net.push_back(block_id);
            }
        }
        
        // Build Q matrix entries for moveable blocks (clique edges)
        for (size_t i = 0; i < moveable_in_net.size(); i++) {
            int block_i = moveable_in_net[i];
            int idx_i = block_to_index[block_i];
            
            for (size_t j = i + 1; j < moveable_in_net.size(); j++) {
                int block_j = moveable_in_net[j];
                int idx_j = block_to_index[block_j];
                
                // Add edge weight to diagonal entries
                Q_coo[{idx_i, idx_i}] += weight;
                Q_coo[{idx_j, idx_j}] += weight;
                
                // Add negative weight to off-diagonal entries
                Q_coo[{idx_i, idx_j}] -= weight;
                Q_coo[{idx_j, idx_i}] -= weight;
            }
        }
        
        // Build b vector contributions from fixed blocks
        for (int fixed_id : fixed_in_net) {
            double fixed_x = blocks[fixed_id].x;
            double fixed_y = blocks[fixed_id].y;
            
            for (int moveable_id : moveable_in_net) {
                int idx = block_to_index[moveable_id];
                
                // Each moveable block connected to fixed block
                // contributes weight * fixed_position to b
                b_x[idx] += weight * fixed_x;
                b_y[idx] += weight * fixed_y;
                
                // Also add to diagonal of Q
                Q_coo[{idx, idx}] += weight;
            }
        }
        
        nets_processed++;
    }
    
    std::cout << "  Processed " << nets_processed << " nets" << std::endl;
    std::cout << "  Non-zero entries in Q: " << Q_coo.size() << std::endl;
    
    // Step 4: Convert COO to compressed column format
    Q.n = n;
    Q.nz = Q_coo.size();
    Q.Ap = new int[n + 1];
    Q.Ai = new int[Q.nz];
    Q.Ax = new double[Q.nz];
    
    // Sort entries by column, then by row
    std::vector<std::tuple<int, int, double>> entries;  // (col, row, value)
    for (const auto& [pos, value] : Q_coo) {
        int row = pos.first;
        int col = pos.second;
        entries.push_back({col, row, value});
    }
    std::sort(entries.begin(), entries.end());
    
    // Fill compressed column arrays
    Q.Ap[0] = 0;
    int current_col = 0;
    int entry_idx = 0;
    
    for (const auto& [col, row, value] : entries) {
        // Fill column pointers for any empty columns
        while (current_col < col) {
            current_col++;
            Q.Ap[current_col] = entry_idx;
        }
        
        Q.Ai[entry_idx] = row;
        Q.Ax[entry_idx] = value;
        entry_idx++;
    }
    
    // Fill remaining column pointers
    while (current_col < n) {
        current_col++;
        Q.Ap[current_col] = entry_idx;
    }
    
    std::cout << "\n[SUCCESS] Placement system built" << std::endl;
    std::cout << "  Matrix size: " << n << " x " << n << std::endl;
    std::cout << "  Non-zeros: " << Q.nz << std::endl;
    std::cout << "  Sparsity: " << std::fixed << std::setprecision(2) 
              << (100.0 * Q.nz / (n * n)) << "%" << std::endl;
    
    return true;
}

void free_sparse_matrix(SparseMatrix& mat) {
    if (mat.Ap) delete[] mat.Ap;
    if (mat.Ai) delete[] mat.Ai;
    if (mat.Ax) delete[] mat.Ax;
    
    mat.Ap = nullptr;
    mat.Ai = nullptr;
    mat.Ax = nullptr;
    mat.n = 0;
    mat.nz = 0;
}

void print_matrix_info(const SparseMatrix& mat) {
    std::cout << "\n========================================" << std::endl;
    std::cout << "      Sparse Matrix Information" << std::endl;
    std::cout << "========================================" << std::endl;
    std::cout << "Dimension: " << mat.n << " x " << mat.n << std::endl;
    std::cout << "Non-zeros: " << mat.nz << std::endl;
    std::cout << "Sparsity:  " << std::fixed << std::setprecision(2)
              << (100.0 * mat.nz / (mat.n * mat.n)) << "%" << std::endl;
    
    // Print first few entries for debugging
    std::cout << "\nFirst 10 entries (column format):" << std::endl;
    std::cout << "  Col  Row  Value" << std::endl;
    
    int count = 0;
    for (int col = 0; col < mat.n && count < 10; col++) {
        for (int idx = mat.Ap[col]; idx < mat.Ap[col + 1] && count < 10; idx++) {
            std::cout << "  " << std::setw(3) << col 
                      << "  " << std::setw(3) << mat.Ai[idx]
                      << "  " << std::setw(8) << std::setprecision(4) 
                      << mat.Ax[idx] << std::endl;
            count++;
        }
    }
    
    if (mat.nz > 10) {
        std::cout << "  ... (" << (mat.nz - 10) << " more entries)" << std::endl;
    }
    
    std::cout << "========================================\n" << std::endl;
}
#include <iostream>
#include <cstring>
#include "data_structures.h"
#include "input_parser.h"
#include "weight_matrix.h"
#include "umfpack_solver.h"
#include "hpwl_calculator.h"
#include "graphics.h"
#include "spreader.h"

int main(int argc, char* argv[]) {
    
    // Parse command line arguments
    if (argc == 1) {
        std::cerr << "Error: Circuit file (-f) is required" << std::endl;
        return 1;
    }
    
    std::string circuit_file;
    bool enable_graphics = false;
    bool enable_spreading = false;
    SpreadMode spread_mode = SpreadMode::HOMOGENEOUS;
    
    // Part 2i parameters (psi strategy)
    double psi_init = 2.0;      // Default: medium start
    double psi_incr = 1.0;      // Default: linear growth
    
    // Part 2ii parameter (anchoring)
    double anchor_weight = -1.0;  // -1 = disabled, >0 = enabled
    
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0) {
            return 0;
        }
        else if (strcmp(argv[i], "-f") == 0) {
            if (i + 1 < argc) {
                circuit_file = argv[++i];
            } else {
                std::cerr << "Error: -f requires a filename argument" << std::endl;
                return 1;
            }
        }
        else if (strcmp(argv[i], "-s") == 0) {
            if (i + 1 < argc) {
                std::string mode = argv[++i];
                if (mode == "ho") {
                    enable_spreading = true;
                    spread_mode = SpreadMode::HOMOGENEOUS;
                } else if (mode == "he") {
                    enable_spreading = true;
                    spread_mode = SpreadMode::HETEROGENEOUS;
                } else {
                    std::cerr << "Error: -s requires 'ho' or 'he'" << std::endl;
                    return 1;
                }
            } else {
                std::cerr << "Error: -s requires a mode argument (ho or he)" << std::endl;
                return 1;
            }
        }
        else if (strcmp(argv[i], "--psi-init") == 0) {
            if (i + 1 < argc) {
                psi_init = std::atof(argv[++i]);
                if (psi_init <= 0) {
                    std::cerr << "Error: --psi-init must be positive" << std::endl;
                    return 1;
                }
            } else {
                std::cerr << "Error: --psi-init requires a value" << std::endl;
                return 1;
            }
        }
        else if (strcmp(argv[i], "--psi-incr") == 0) {
            if (i + 1 < argc) {
                psi_incr = std::atof(argv[++i]);
                if (psi_incr <= 0) {
                    std::cerr << "Error: --psi-incr must be positive" << std::endl;
                    return 1;
                }
            } else {
                std::cerr << "Error: --psi-incr requires a value" << std::endl;
                return 1;
            }
        }
        else if (strcmp(argv[i], "--anchor-weight") == 0) {
            if (i + 1 < argc) {
                anchor_weight = std::atof(argv[++i]);
                if (anchor_weight <= 0) {
                    std::cerr << "Error: --anchor-weight must be positive" << std::endl;
                    return 1;
                }
            } else {
                std::cerr << "Error: --anchor-weight requires a value" << std::endl;
                return 1;
            }
        }
        else if (strcmp(argv[i], "-g") == 0) {
            enable_graphics = true;
        }
        else {
            std::cerr << "Error: Unknown argument '" << argv[i] << "'" << std::endl;
            return 1;
        }
    }
    
    if (circuit_file.empty()) {
        std::cerr << "Error: Circuit file (-f) is required" << std::endl;
        return 1;
    }
    
    // Validation: anchor-weight requires spreading
    if (anchor_weight > 0 && !enable_spreading) {
        std::cerr << "Error: --anchor-weight requires spreading (-s ho or -s he)" << std::endl;
        return 1;
    }
    
    // Step 1: Parse the input file
    if (!parse_input_file(circuit_file)) {
        std::cerr << "\n[ERROR] Failed to parse input file" << std::endl;
        return 1;
    }
    
    print_circuit_info();
    
    // Step 2: Build weight matrix and b vectors
    SparseMatrix Q;
    std::vector<double> b_x, b_y;
    
    if (!build_placement_system(Q, b_x, b_y)) {
        std::cerr << "\n[ERROR] Failed to build placement system" << std::endl;
        return 1;
    }
    
    // Step 3: Solve placement with UMFPACK
    if (!solve_placement(Q, b_x, b_y)) {
        std::cerr << "\n[ERROR] Failed to solve placement" << std::endl;
        free_sparse_matrix(Q);
        return 1;
    }
    
    // Step 4: Calculate and print HPWL (Part 1)
    std::cout << "\n========================================" << std::endl;
    std::cout << "     Part 1: Analytical Placement" << std::endl;
    std::cout << "========================================" << std::endl;
    print_hpwl_stats();
    
    // Step 5: Apply spreading if requested (Part 2i)
    SpreadStats stats;
    if (enable_spreading) {
        std::cout << "\n========================================" << std::endl;
        std::cout << "     Part 2: Flow-Based Spreading" << std::endl;
        std::cout << "========================================" << std::endl;
        
        stats = apply_flow_based_spreading(spread_mode, psi_init, psi_incr);
        print_spread_stats(stats, spread_mode);
        
        // Step 6: Apply anchoring if requested (Part 2ii)
        if (anchor_weight > 0) {
            std::cout << "\n========================================" << std::endl;
            std::cout << "     Part 2ii: Anchoring & Re-solve" << std::endl;
            std::cout << "========================================" << std::endl;
            
            std::cout << "\nAnchor weight: " << anchor_weight << std::endl;
            
            // Save HPWL after spreading (before anchoring)
            double hpwl_after_spread = calculate_hpwl();
            
            // Create anchors at current (spread) positions
            create_anchors_and_pseudo_nets(anchor_weight);
            
            // Rebuild weight matrix (now includes pseudo nets)
            std::cout << "\nRebuilding placement system with anchors..." << std::endl;
            SparseMatrix Q2;
            std::vector<double> b_x2, b_y2;
            if (!build_placement_system(Q2, b_x2, b_y2)) {
                std::cerr << "[ERROR] Failed to rebuild placement system" << std::endl;
                free_sparse_matrix(Q);
                return 1;
            }
            
            // Re-solve placement with anchors
            std::cout << "Re-solving placement with anchors..." << std::endl;
            if (!solve_placement(Q2, b_x2, b_y2)) {
                std::cerr << "[ERROR] Failed to re-solve placement" << std::endl;
                free_sparse_matrix(Q);
                free_sparse_matrix(Q2);
                return 1;
            }
            
            // Calculate final HPWL
            double hpwl_after_anchor = calculate_hpwl();
            
            std::cout << "\n========================================" << std::endl;
            std::cout << "    HPWL Results (Part 2ii)" << std::endl;
            std::cout << "========================================" << std::endl;
            std::cout << "\nAfter analytical placement: " << stats.hpwl_before << std::endl;
            std::cout << "After spreading:            " << hpwl_after_spread << std::endl;
            std::cout << "After anchoring & re-solve: " << hpwl_after_anchor << std::endl;
            
            double change_spread = hpwl_after_spread - stats.hpwl_before;
            double change_anchor = hpwl_after_anchor - hpwl_after_spread;
            double change_total = hpwl_after_anchor - stats.hpwl_before;
            
            std::cout << "\nChange from spreading:      " << std::showpos << change_spread 
                      << " (" << (change_spread / stats.hpwl_before * 100.0) << "%)" << std::noshowpos << std::endl;
            std::cout << "Change from anchoring:      " << std::showpos << change_anchor 
                      << " (" << (change_anchor / hpwl_after_spread * 100.0) << "%)" << std::noshowpos << std::endl;
            std::cout << "Total change:               " << std::showpos << change_total 
                      << " (" << (change_total / stats.hpwl_before * 100.0) << "%)" << std::noshowpos << std::endl;
            
            std::cout << "========================================\n" << std::endl;
            
            free_sparse_matrix(Q2);
        }
    }
    
    // Cleanup
    free_sparse_matrix(Q);
    
    // Step 7: Display graphics if requested
    if (enable_graphics) {
        run_graphics();  // This is a blocking calls
    }
    
    std::cout << "\n========================================" << std::endl;
    std::cout << "         Completion Status" << std::endl;
    std::cout << "========================================" << std::endl;
    std::cout << "  [✓] Parse input file" << std::endl;
    std::cout << "  [✓] Build weight matrix (clique model)" << std::endl;
    std::cout << "  [✓] Solve linear system with UMFPACK" << std::endl;
    std::cout << "  [✓] Calculate HPWL" << std::endl;
    if (enable_spreading) {
        std::cout << "  [✓] Apply flow-based spreading" << std::endl;
    }
    if (anchor_weight > 0) {
        std::cout << "  [✓] Apply anchoring & re-solve" << std::endl;
    }
    std::cout << "  [✓] Display with EZGL graphics" << std::endl;
    std::cout << "========================================\n" << std::endl;
    
    std::cout << "[SUCCESS] Placement complete!" << std::endl;
    
    return 0;
}
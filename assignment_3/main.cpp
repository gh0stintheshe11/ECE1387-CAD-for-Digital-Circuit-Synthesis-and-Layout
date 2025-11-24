#include <iostream>
#include <chrono>
#include <cmath>
#include <cstring>
#include "parser.h"
#include "partitioner.h"
#include "graphics.h"

void print_usage(const char* prog_name) {
    std::cerr << "Usage: " << prog_name << " -f <circuit_file> [-t num_threads] [-g]" << std::endl;
    std::cerr << "  -f circuit_file : input circuit file (required)" << std::endl;
    std::cerr << "  -t num_threads  : power of 2 (1, 2, 4, 8, 16, ...), default: 4" << std::endl;
    std::cerr << "  -g              : show graphics visualization (requires -t 1)" << std::endl;
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        print_usage(argv[0]);
        return 1;
    }
    
    std::string filename;
    int num_threads = 4;  // Default
    bool show_graphics = false;
    
    // Parse arguments
    int i = 1;
    while (i < argc) {
        if (strcmp(argv[i], "-f") == 0) {
            if (i + 1 >= argc) {
                std::cerr << "Error: -f requires a filename" << std::endl;
                print_usage(argv[0]);
                return 1;
            }
            filename = argv[i + 1];
            i += 2;
        } else if (strcmp(argv[i], "-t") == 0) {
            if (i + 1 >= argc) {
                std::cerr << "Error: -t requires a number" << std::endl;
                print_usage(argv[0]);
                return 1;
            }
            num_threads = std::stoi(argv[i + 1]);
            i += 2;
        } else if (strcmp(argv[i], "-g") == 0) {
            show_graphics = true;
            i += 1;
        } else {
            std::cerr << "Error: Unknown option " << argv[i] << std::endl;
            print_usage(argv[0]);
            return 1;
        }
    }
    
    if (filename.empty()) {
        std::cerr << "Error: No circuit file specified (use -f)" << std::endl;
        print_usage(argv[0]);
        return 1;
    }
    
    // Validate num_threads is power of 2
    if (num_threads < 1 || (num_threads & (num_threads - 1)) != 0) {
        std::cerr << "Error: num_threads must be a power of 2 (1, 2, 4, 8, 16, ...)" << std::endl;
        return 1;
    }
    
    // Graphics requires sequential mode
    if (show_graphics && num_threads != 1) {
        std::cerr << "Warning: Graphics requires sequential mode. Setting -t 1" << std::endl;
        num_threads = 1;
    }
    
    // Enable graphics recording if requested
    g_graphics_enabled = show_graphics;
    
    // Parse circuit
    std::cout << "Parsing circuit file: " << filename << std::endl;
    Circuit circuit = parse_circuit(filename);
    
    // Print summary
    print_circuit_summary(circuit);
    
    // Run partitioner with timing
    auto start = std::chrono::high_resolution_clock::now();
    
    PartitionResult result = partition(circuit, num_threads);
    
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    
    std::cout << "Runtime: " << duration.count() << " ms" << std::endl;
    
    // Print partition details
    std::cout << "\nLeft partition (" << result.left_partition.size() << " blocks): ";
    for (int blk : result.left_partition) {
        std::cout << blk << " ";
    }
    std::cout << std::endl;
    
    std::cout << "Right partition (" << result.right_partition.size() << " blocks): ";
    for (int blk : result.right_partition) {
        std::cout << blk << " ";
    }
    std::cout << std::endl;
    
    // Launch graphics if requested
    if (show_graphics) {
        run_graphics(filename, result.total_cost, result.nodes_visited);
    }
    
    return 0;
}
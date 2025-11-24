#include <iostream>
#include <chrono>
#include "parser.h"
#include "partitioner.h"

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <circuit_file>" << std::endl;
        return 1;
    }
    
    std::string filename = argv[1];
    
    // Parse circuit
    std::cout << "Parsing circuit file: " << filename << std::endl;
    Circuit circuit = parse_circuit(filename);
    
    // Print summary
    print_circuit_summary(circuit);
    
    // Run partitioner with timing
    auto start = std::chrono::high_resolution_clock::now();
    
    PartitionResult result = partition(circuit);
    
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
    
    return 0;
}
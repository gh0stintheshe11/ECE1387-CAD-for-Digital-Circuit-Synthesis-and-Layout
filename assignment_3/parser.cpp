#include "parser.h"
#include <iostream>
#include <fstream>
#include <sstream>

Circuit parse_circuit(const std::string& filename) {
    Circuit circuit;
    std::ifstream infile(filename);
    
    if (!infile.is_open()) {
        std::cerr << "Error: Cannot open file " << filename << std::endl;
        exit(1);
    }
    
    std::string line;
    bool parsing_blocks = true;  // First section is blocks, then community pairs
    
    while (std::getline(infile, line)) {
        // Skip empty lines
        if (line.empty()) continue;
        
        std::istringstream iss(line);
        int first_num;
        iss >> first_num;
        
        // Check for section terminator
        if (first_num == -1) {
            if (parsing_blocks) {
                // End of blocks section, start community section
                parsing_blocks = false;
                continue;
            } else {
                // End of community section, done parsing
                break;
            }
        }
        
        if (parsing_blocks) {
            // Parsing block line: blocknum net1 net2 ... -1
            int block_id = first_num;
            circuit.block_ids.push_back(block_id);
            
            int net_id;
            while (iss >> net_id && net_id != -1) {
                circuit.block_to_nets[block_id].push_back(net_id);
                circuit.net_to_blocks[net_id].push_back(block_id);
                circuit.net_ids.insert(net_id);
            }
        } else {
            // Parsing community pair: block_i block_j
            int block_i = first_num;
            int block_j;
            iss >> block_j;
            circuit.community_pairs.push_back({block_i, block_j});
        }
    }
    
    infile.close();
    
    // Set counts
    circuit.num_blocks = circuit.block_ids.size();
    circuit.num_nets = circuit.net_ids.size();
    
    return circuit;
}

void print_circuit_summary(const Circuit& circuit) {
    std::cout << "=== Circuit Summary ===" << std::endl;
    std::cout << "Blocks: " << circuit.num_blocks << std::endl;
    std::cout << "Nets: " << circuit.num_nets << std::endl;
    std::cout << "Community pairs: " << circuit.community_pairs.size() << std::endl;
    
    // Compute some statistics
    int total_pins = 0;
    int max_block_fanout = 0;
    int max_net_degree = 0;
    
    for (const auto& [block_id, nets] : circuit.block_to_nets) {
        total_pins += nets.size();
        if ((int)nets.size() > max_block_fanout) {
            max_block_fanout = nets.size();
        }
    }
    
    for (const auto& [net_id, blocks] : circuit.net_to_blocks) {
        if ((int)blocks.size() > max_net_degree) {
            max_net_degree = blocks.size();
        }
    }
    
    std::cout << "Total pins: " << total_pins << std::endl;
    std::cout << "Max block fanout: " << max_block_fanout << std::endl;
    std::cout << "Max net degree: " << max_net_degree << std::endl;
}
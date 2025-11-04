#include "input_parser.h"
#include "data_structures.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <iomanip>

// Define global variables
std::map<int, Block> blocks;
std::map<int, Net> nets;
double global_anchor_weight_scale = 1.0;

bool parse_input_file(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Error: Cannot open file '" << filename << "'" << std::endl;
        return false;
    }
    
    std::cout << "\nParsing input file: " << filename << std::endl;
    
    // Clear any existing data
    blocks.clear();
    nets.clear();
    
    // Parse moveable blocks
    std::string line;
    int line_num = 0;
    
    std::cout << "Reading moveable blocks" << std::endl;
    
    while (std::getline(file, line)) {
        line_num++;
        std::istringstream iss(line);
        int blocknum;
        
        // Skip empty lines
        if (!(iss >> blocknum)) continue;
        
        // Check for section separator
        if (blocknum == -1) {
            std::cout << "End of moveable blocks (line " << line_num << ")" << std::endl;
            break;
        }
        
        // Parse: blocknum blocktype netnum1 netnum2 ... -1
        Block block;
        block.id = blocknum;
        block.is_fixed = false;
        block.x = 0.0;  // Will be set by solver or fixed position
        block.y = 0.0;
        
        if (!(iss >> block.type)) {
            std::cerr << "Error: Missing block type at line " << line_num << std::endl;
            return false;
        }
        
        // Validate block type
        if (block.type != 0 && block.type != 1) {
            std::cerr << "Error: Invalid block type " << block.type 
                      << " at line " << line_num << " (must be 0 or 1)" << std::endl;
            return false;
        }
        
        // Read net numbers until -1
        int netnum;
        while (iss >> netnum) {
            if (netnum == -1) break;
            
            block.nets.push_back(netnum);
            
            // Also add this block to the net
            nets[netnum].id = netnum;
            nets[netnum].blocks.push_back(blocknum);
        }
        
        blocks[blocknum] = block;
    }
    
    // Parse fixed blocks
    std::cout << "Reading fixed I/O blocks" << std::endl;
    
    while (std::getline(file, line)) {
        line_num++;
        std::istringstream iss(line);
        int blocknum;
        
        // Skip empty lines
        if (!(iss >> blocknum)) continue;
        
        // Check for end of section
        if (blocknum == -1) {
            std::cout << "End of fixed blocks (line " << line_num << ")" << std::endl;
            break;
        }
        
        double x, y;
        if (!(iss >> x >> y)) {
            std::cerr << "Error: Missing x,y coordinates for fixed block at line " 
                      << line_num << std::endl;
            return false;
        }
        
        // Mark block as fixed and set position
        if (blocks.find(blocknum) != blocks.end()) {
            blocks[blocknum].is_fixed = true;
            blocks[blocknum].x = x;
            blocks[blocknum].y = y;
        } else {
            std::cerr << "Warning: Fixed block " << blocknum 
                      << " (line " << line_num << ") not found in moveable blocks!" 
                      << std::endl;
        }
    }
    
    file.close();
    
    std::cout << "Parsed " << blocks.size() << " blocks and " 
              << nets.size() << " nets" << std::endl;
    
    return true;
}

void print_circuit_info() {
    std::cout << "\nCircuit Information" << std::endl;
    
    // Count fixed vs moveable and types
    int fixed_count = 0;
    int type0_count = 0;
    int type1_count = 0;
    
    for (const auto& [id, block] : blocks) {
        if (block.is_fixed) fixed_count++;
        if (block.type == 0) type0_count++;
        else if (block.type == 1) type1_count++;
    }
    
    std::cout << "Summary:" << std::endl;
    std::cout << "  Total blocks:      " << blocks.size() << std::endl;
    std::cout << "    Fixed blocks:    " << fixed_count << std::endl;
    std::cout << "    Moveable blocks: " << (blocks.size() - fixed_count) << std::endl;
    std::cout << "    Type 0 blocks:   " << type0_count << std::endl;
    std::cout << "    Type 1 blocks:   " << type1_count << std::endl;
    std::cout << "  Total nets:        " << nets.size() << std::endl;
    
    // Calculate average pins per net
    int total_pins = 0;
    for (const auto& [id, net] : nets) {
        total_pins += net.blocks.size();
    }
    double avg_pins = nets.empty() ? 0.0 : (double)total_pins / nets.size();
    std::cout << "  Avg pins per net:  " << std::fixed << std::setprecision(2) 
              << avg_pins << std::endl;
}
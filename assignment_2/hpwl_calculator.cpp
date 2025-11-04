#include "hpwl_calculator.h"
#include "data_structures.h"
#include <iostream>
#include <iomanip>
#include <limits>
#include <vector>
#include <algorithm>

double calculate_hpwl() {
    double total_hpwl = 0.0;
    
    // Process each net
    for (const auto& [net_id, net] : nets) {
        if (net.blocks.size() < 2) {
            // Single-pin nets contribute 0 to HPWL
            continue;
        }
        
        // Initialize bounding box to extreme values
        double min_x = std::numeric_limits<double>::max();
        double max_x = std::numeric_limits<double>::lowest();
        double min_y = std::numeric_limits<double>::max();
        double max_y = std::numeric_limits<double>::lowest();
        
        // Find bounding box for this net
        for (int block_id : net.blocks) {
            const Block& block = blocks[block_id];
            double x = block.x;
            double y = block.y;
            
            if (x < min_x) min_x = x;
            if (x > max_x) max_x = x;
            if (y < min_y) min_y = y;
            if (y > max_y) max_y = y;
        }
        
        // Calculate HPWL for this net (half-perimeter of bounding box)
        double net_hpwl = (max_x - min_x) + (max_y - min_y);
        total_hpwl += net_hpwl;
    }
    
    return total_hpwl;
}

void print_hpwl_stats() {
    std::cout << "\n========================================" << std::endl;
    std::cout << "      HPWL (Wirelength) Analysis" << std::endl;
    std::cout << "========================================" << std::endl;
    
    double total_hpwl = 0.0;
    int num_nets_counted = 0;
    
    // Statistics
    double min_net_hpwl = std::numeric_limits<double>::max();
    double max_net_hpwl = 0.0;
    std::vector<double> net_hpwls;
    
    // Calculate HPWL for each net
    for (const auto& [net_id, net] : nets) {
        if (net.blocks.size() < 2) {
            continue;  // Skip single-pin nets
        }
        
        // Find bounding box
        double min_x = std::numeric_limits<double>::max();
        double max_x = std::numeric_limits<double>::lowest();
        double min_y = std::numeric_limits<double>::max();
        double max_y = std::numeric_limits<double>::lowest();
        
        for (int block_id : net.blocks) {
            const Block& block = blocks[block_id];
            double x = block.x;
            double y = block.y;
            
            if (x < min_x) min_x = x;
            if (x > max_x) max_x = x;
            if (y < min_y) min_y = y;
            if (y > max_y) max_y = y;
        }
        
        // Calculate HPWL for this net
        double net_hpwl = (max_x - min_x) + (max_y - min_y);
        total_hpwl += net_hpwl;
        net_hpwls.push_back(net_hpwl);
        num_nets_counted++;
        
        if (net_hpwl < min_net_hpwl) min_net_hpwl = net_hpwl;
        if (net_hpwl > max_net_hpwl) max_net_hpwl = net_hpwl;
    }
    
    // Calculate average
    double avg_net_hpwl = (num_nets_counted > 0) ? (total_hpwl / num_nets_counted) : 0.0;
    
    // Calculate median
    double median_net_hpwl = 0.0;
    if (!net_hpwls.empty()) {
        std::sort(net_hpwls.begin(), net_hpwls.end());
        size_t mid = net_hpwls.size() / 2;
        if (net_hpwls.size() % 2 == 0) {
            median_net_hpwl = (net_hpwls[mid - 1] + net_hpwls[mid]) / 2.0;
        } else {
            median_net_hpwl = net_hpwls[mid];
        }
    }
    
    // Print statistics
    std::cout << std::fixed << std::setprecision(2);
    std::cout << "\nTotal HPWL:        " << total_hpwl << std::endl;
    std::cout << "Nets counted:      " << num_nets_counted << std::endl;
    std::cout << "Average net HPWL:  " << avg_net_hpwl << std::endl;
    std::cout << "Median net HPWL:   " << median_net_hpwl << std::endl;
    std::cout << "Min net HPWL:      " << min_net_hpwl << std::endl;
    std::cout << "Max net HPWL:      " << max_net_hpwl << std::endl;
    
    // Print worst 5 nets (highest HPWL)
    std::cout << "\n=== Top 5 Longest Nets ===" << std::endl;
    
    // Create vector of (net_id, hpwl) pairs
    std::vector<std::pair<int, double>> net_hpwl_pairs;
    for (const auto& [net_id, net] : nets) {
        if (net.blocks.size() < 2) continue;
        
        double min_x = std::numeric_limits<double>::max();
        double max_x = std::numeric_limits<double>::lowest();
        double min_y = std::numeric_limits<double>::max();
        double max_y = std::numeric_limits<double>::lowest();
        
        for (int block_id : net.blocks) {
            const Block& block = blocks[block_id];
            if (block.x < min_x) min_x = block.x;
            if (block.x > max_x) max_x = block.x;
            if (block.y < min_y) min_y = block.y;
            if (block.y > max_y) max_y = block.y;
        }
        
        double hpwl = (max_x - min_x) + (max_y - min_y);
        net_hpwl_pairs.push_back({net_id, hpwl});
    }
    
    // Sort by HPWL (descending)
    std::sort(net_hpwl_pairs.begin(), net_hpwl_pairs.end(),
              [](const auto& a, const auto& b) { return a.second > b.second; });
    
    // Print top 5
    for (int i = 0; i < std::min(5, (int)net_hpwl_pairs.size()); i++) {
        int net_id = net_hpwl_pairs[i].first;
        double hpwl = net_hpwl_pairs[i].second;
        int num_pins = nets.at(net_id).blocks.size();
        
        std::cout << "  Net " << std::setw(4) << net_id 
                  << " (" << std::setw(2) << num_pins << " pins): HPWL = " 
                  << std::setw(8) << hpwl << std::endl;
    }
    
    std::cout << "========================================\n" << std::endl;
}
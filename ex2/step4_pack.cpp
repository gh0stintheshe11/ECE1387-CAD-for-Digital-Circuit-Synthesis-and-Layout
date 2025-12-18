// step4_pack.cpp
// Fracturable LUT Packing Program
// Compile: g++ -std=c++11 -O2 step4_pack.cpp -o step4_pack

#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <set>
#include <string>
#include <algorithm>
#include <iomanip>
#include <cstdio>
#include <sys/stat.h>

using namespace std;

// Global log file
ofstream logfile;

// Helper function to output to both console and log file
void log_output(const string& str) {
    cout << str;
    if (logfile.is_open()) {
        logfile << str;
    }
}

struct LUT {
    string output;
    set<string> inputs;
    int num_inputs;
};

// Parse BLIF file and extract LUT information
vector<LUT> parse_blif(const string& filename) {
    vector<LUT> luts;
    ifstream infile(filename);
    
    if (!infile.is_open()) {
        cerr << "Error: Cannot open file " << filename << endl;
        return luts;
    }
    
    string line;
    while (getline(infile, line)) {
        // Remove leading/trailing whitespace
        size_t start = line.find_first_not_of(" \t\r\n");
        if (start == string::npos) continue;
        line = line.substr(start);
        
        // Look for .names lines
        if (line.substr(0, 6) == ".names") {
            istringstream iss(line);
            string token;
            vector<string> tokens;
            
            while (iss >> token) {
                tokens.push_back(token);
            }
            
            // tokens[0] is ".names", last token is output, rest are inputs
            if (tokens.size() >= 2) {
                LUT lut;
                lut.output = tokens.back();
                
                // Get all inputs (skip first token ".names" and last token which is output)
                for (size_t i = 1; i < tokens.size() - 1; i++) {
                    lut.inputs.insert(tokens[i]);
                }
                
                lut.num_inputs = lut.inputs.size();
                
                // Skip constant generators (no inputs)
                if (lut.num_inputs > 0) {
                    luts.push_back(lut);
                }
            }
        }
    }
    
    infile.close();
    return luts;
}

// Check if two LUTs can be packed together
bool can_pack_together(const LUT& lut1, const LUT& lut2) {
    set<string> combined_inputs;
    set_union(lut1.inputs.begin(), lut1.inputs.end(),
              lut2.inputs.begin(), lut2.inputs.end(),
              inserter(combined_inputs, combined_inputs.begin()));
    
    return combined_inputs.size() <= 5;
}

// Greedy packing algorithm
vector<vector<string>> greedy_pack(const vector<LUT>& luts) {
    vector<vector<string>> packed;
    vector<bool> used(luts.size(), false);
    
    // Build compatibility list for each LUT
    vector<vector<int>> compatibility(luts.size());
    for (size_t i = 0; i < luts.size(); i++) {
        for (size_t j = i + 1; j < luts.size(); j++) {
            if (can_pack_together(luts[i], luts[j])) {
                compatibility[i].push_back(j);
                compatibility[j].push_back(i);
            }
        }
    }
    
    // Create sorted index list (prioritize LUTs with fewer compatible partners)
    vector<int> indices(luts.size());
    for (size_t i = 0; i < luts.size(); i++) {
        indices[i] = i;
    }
    sort(indices.begin(), indices.end(), 
         [&compatibility](int a, int b) {
             return compatibility[a].size() < compatibility[b].size();
         });
    
    // Greedy pairing
    for (int i : indices) {
        if (used[i]) continue;
        
        // Try to find a compatible partner
        int best_partner = -1;
        for (int j : compatibility[i]) {
            if (!used[j]) {
                best_partner = j;
                break;
            }
        }
        
        if (best_partner != -1) {
            // Pack two LUTs together
            packed.push_back({luts[i].output, luts[best_partner].output});
            used[i] = true;
            used[best_partner] = true;
        } else {
            // Pack single LUT
            packed.push_back({luts[i].output});
            used[i] = true;
        }
    }
    
    return packed;
}

// Write output to file
void write_output(const vector<vector<string>>& packed, const string& output_filename) {
    ofstream outfile(output_filename);
    
    for (const auto& pack : packed) {
        for (size_t i = 0; i < pack.size(); i++) {
            outfile << pack[i];
            if (i < pack.size() - 1) outfile << " ";
        }
        outfile << "\n";
    }
    
    outfile.close();
}

int main() {
    vector<string> circuits = {"alu4", "clma", "div", "misex3", "sqrt"};
    
    // Create step_4 directory
    mkdir("step_4", 0755);
    
    // Open log file
    logfile.open("step_4/step4_log.txt");
    
    log_output("Step 4: Fracturable LUT Packing\n");
    log_output(string(50, '=') + "\n");
    log_output("\n");
    
    struct Result {
        string circuit;
        int original;
        int fracturable;
        int reduction;
    };
    vector<Result> results;
    
    for (const string& circuit : circuits) {
        string input_file = "step_2/" + circuit + ".mapped.blif";
        string output_file = "step_4/" + circuit + ".packed.txt";
        
        log_output("Processing " + circuit + "...\n");
        
        // Parse BLIF
        vector<LUT> luts = parse_blif(input_file);
        int original_lut_count = luts.size();
        
        // Pack LUTs
        vector<vector<string>> packed = greedy_pack(luts);
        int fracturable_lut_count = packed.size();
        
        // Count paired vs single
        int paired_count = 0, single_count = 0;
        for (const auto& pack : packed) {
            if (pack.size() == 2) paired_count++;
            else if (pack.size() == 1) single_count++;
        }
        
        // Write output
        write_output(packed, output_file);
        
        int reduction = original_lut_count - fracturable_lut_count;
        double percent = 100.0 * reduction / original_lut_count;
        
        ostringstream oss;
        oss << "  Original LUTs: " << original_lut_count << "\n";
        oss << "  Fracturable LUTs: " << fracturable_lut_count << "\n";
        oss << "  Paired: " << paired_count << ", Single: " << single_count << "\n";
        oss << "  Reduction: " << reduction << " LUTs (" 
            << fixed << setprecision(1) << percent << "%)\n\n";
        
        log_output(oss.str());
        
        results.push_back({circuit, original_lut_count, fracturable_lut_count, reduction});
    }
    
    // Summary
    log_output(string(50, '=') + "\n");
    log_output("Summary:\n");
    
    char buffer[256];
    snprintf(buffer, sizeof(buffer), "%-10s %-10s %-12s %-10s\n", 
             "Circuit", "Original", "Fracturable", "Reduction");
    log_output(string(buffer));
    log_output(string(50, '-') + "\n");
    
    int total_original = 0, total_fracturable = 0, total_reduction = 0;
    for (const auto& r : results) {
        snprintf(buffer, sizeof(buffer), "%-10s %-10d %-12d %-10d\n", 
                r.circuit.c_str(), r.original, r.fracturable, r.reduction);
        log_output(string(buffer));
        
        total_original += r.original;
        total_fracturable += r.fracturable;
        total_reduction += r.reduction;
    }
    
    log_output(string(50, '-') + "\n");
    double total_percent = 100.0 * total_reduction / total_original;
    snprintf(buffer, sizeof(buffer), "%-10s %-10d %-12d %-10d (%.1f%%)\n", 
            "TOTAL", total_original, total_fracturable, total_reduction, total_percent);
    log_output(string(buffer));
    
    // Close log file
    logfile.close();
    
    cout << "\nResults saved to step_4/step4_log.txt" << endl;
    
    return 0;
}
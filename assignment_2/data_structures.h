#ifndef DATA_STRUCTURES_H
#define DATA_STRUCTURES_H

#include <vector>
#include <map>
#include <string>
#include <algorithm>

// Block/Cell structure
struct Block {
    int id;
    int type;                    // 0 or 1
    double x, y;                 // position
    bool is_fixed;
    bool is_anchor;              // for spreading: true for anchor blocks
    std::vector<int> nets;       // nets connected to this block
};

// Net structure
struct Net {
    int id;
    std::vector<int> blocks;     // blocks connected to this net
    bool is_pseudo;              // for anchor pseudo nets
};

// Bin structure (for spreading algorithm)
struct Bin {
    int col;           // Column index (0-39)
    int row;           // Row index (0-39)
    double x_min, x_max;  // Bin boundaries
    double y_min, y_max;
    double capacity;   // How many cells can fit (1.0 for unit bins)
    std::vector<int> cells;  // Block IDs in this bin
    
    // Default constructor (needed for vector resize)
    Bin() : col(0), row(0), x_min(0), x_max(0), y_min(0), y_max(0), capacity(1.0) {}
    
    // Parameterized constructor
    Bin(int c, int r, double xmin, double xmax, double ymin, double ymax)
        : col(c), row(r), x_min(xmin), x_max(xmax), y_min(ymin), y_max(ymax), capacity(1.0) {}
    
    double demand() const { return static_cast<double>(cells.size()); }
    double overflow() const { return std::max(0.0, demand() - capacity); }
    
    // Get center of bin
    double center_x() const { return (x_min + x_max) / 2.0; }
    double center_y() const { return (y_min + y_max) / 2.0; }
};

// Global circuit data
extern std::map<int, Block> blocks;     // blockID -> Block
extern std::map<int, Net> nets;         // netID -> Net

// Global anchor weight scale (CHANGED: extern only, define in .cpp)
extern double global_anchor_weight_scale;

#endif // DATA_STRUCTURES_H
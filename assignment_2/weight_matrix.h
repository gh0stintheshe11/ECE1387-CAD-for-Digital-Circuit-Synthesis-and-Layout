#ifndef WEIGHT_MATRIX_H
#define WEIGHT_MATRIX_H

#include <vector>

// Sparse matrix in compressed column format (for UMFPACK)
struct SparseMatrix {
    int n;              // Matrix dimension (number of moveable blocks)
    int nz;             // Number of non-zeros
    int* Ap;            // Column pointers [n+1]
    int* Ai;            // Row indices [nz]
    double* Ax;         // Values [nz]
    
    // Constructor
    SparseMatrix() : n(0), nz(0), Ap(nullptr), Ai(nullptr), Ax(nullptr) {}
};

// Build Q matrix and b vectors for analytical placement
// Uses clique net model with weight = 2/p for each edge
bool build_placement_system(
    SparseMatrix& Q,       // Output: Q matrix (same for x and y)
    std::vector<double>& b_x,  // Output: right-hand side for x
    std::vector<double>& b_y   // Output: right-hand side for y
);

// Free allocated memory for sparse matrix
void free_sparse_matrix(SparseMatrix& mat);

// Debug: print matrix statistics
void print_matrix_info(const SparseMatrix& mat);

#endif // WEIGHT_MATRIX_H
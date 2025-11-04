#include "umfpack_solver.h"
#include "data_structures.h"
#include <iostream>
#include <iomanip>
#include <map>
#include <vector>
#include "umfpack.h"

bool solve_placement(
    const SparseMatrix& Q,
    const std::vector<double>& b_x,
    const std::vector<double>& b_y
) {
    std::cout << "\nSolving Placement with UMFPACK" << std::endl;
    
    int n = Q.n;
    
    if (n == 0) {
        std::cerr << "Error: Empty matrix!" << std::endl;
        return false;
    }
    
    if ((int)b_x.size() != n || (int)b_y.size() != n) {
        std::cerr << "Error: Matrix/vector size mismatch!" << std::endl;
        return false;
    }
    
    std::cout << "Matrix dimension: " << n << " x " << n << std::endl;
    std::cout << "Non-zeros: " << Q.nz << std::endl;
    
    // Allocate solution vectors
    double* x_pos = new double[n];
    double* y_pos = new double[n];
    
    // UMFPACK variables
    void *Symbolic, *Numeric;
    double Control[UMFPACK_CONTROL];
    double Info[UMFPACK_INFO];
    
    // Set default control parameters
    umfpack_di_defaults(Control);
    
    // Symbolic factorization
    std::cout << "Symbolic factorization..." << std::endl;
    
    int status = umfpack_di_symbolic(
        n, n,           // Matrix dimensions
        Q.Ap, Q.Ai, Q.Ax,  // Matrix in compressed column format
        &Symbolic,      // Output: symbolic object
        Control, Info
    );
    
    if (status != UMFPACK_OK) {
        std::cerr << "Error in symbolic factorization: " << status << std::endl;
        delete[] x_pos;
        delete[] y_pos;
        return false;
    }
    
    std::cout << "Symbolic factorization successful" << std::endl;
    
    // Numeric factorization
    std::cout << "Numeric factorization" << std::endl;
    
    status = umfpack_di_numeric(
        Q.Ap, Q.Ai, Q.Ax,  // Matrix in compressed column format
        Symbolic,          // Input: symbolic object
        &Numeric,          // Output: numeric object
        Control, Info
    );
    
    if (status != UMFPACK_OK) {
        std::cerr << "Error in numeric factorization: " << status << std::endl;
        umfpack_di_free_symbolic(&Symbolic);
        delete[] x_pos;
        delete[] y_pos;
        return false;
    }
    
    std::cout << "Numeric factorization successful" << std::endl;
    
    // Solve for X coordinates
    std::cout << "Solving for X coordinates" << std::endl;
    
    status = umfpack_di_solve(
        UMFPACK_A,         // Solve Ax=b (not A'x=b)
        Q.Ap, Q.Ai, Q.Ax,  // Matrix
        x_pos,             // Output: solution
        b_x.data(),        // Input: right-hand side
        Numeric,           // Numeric factorization
        Control, Info
    );
    
    if (status != UMFPACK_OK) {
        std::cerr << "Error solving for X: " << status << std::endl;
        umfpack_di_free_symbolic(&Symbolic);
        umfpack_di_free_numeric(&Numeric);
        delete[] x_pos;
        delete[] y_pos;
        return false;
    }
    
    std::cout << "X coordinates solved" << std::endl;
    
    // Solve for Y coordinates
    std::cout << "Solving for Y coordinates" << std::endl;
    
    status = umfpack_di_solve(
        UMFPACK_A,         // Solve Ay=b
        Q.Ap, Q.Ai, Q.Ax,  // Same matrix
        y_pos,             // Output: solution
        b_y.data(),        // Input: right-hand side
        Numeric,           // Same numeric factorization
        Control, Info
    );
    
    if (status != UMFPACK_OK) {
        std::cerr << "Error solving for Y: " << status << std::endl;
        umfpack_di_free_symbolic(&Symbolic);
        umfpack_di_free_numeric(&Numeric);
        delete[] x_pos;
        delete[] y_pos;
        return false;
    }
    
    std::cout << "Y coordinates solved" << std::endl;
    
    // Update block positions
    std::cout << "Updating block positions" << std::endl;
    
    // Create mapping from matrix index to block ID
    std::vector<int> index_to_block;
    for (const auto& [block_id, block] : blocks) {
        if (!block.is_fixed) {
            index_to_block.push_back(block_id);
        }
    }
    
    // Sort to ensure consistent ordering
    std::sort(index_to_block.begin(), index_to_block.end());
    
    // Update positions
    for (int i = 0; i < n; i++) {
        int block_id = index_to_block[i];
        blocks[block_id].x = x_pos[i];
        blocks[block_id].y = y_pos[i];
    }
    
    // Print some statistics
    double min_x = x_pos[0], max_x = x_pos[0];
    double min_y = y_pos[0], max_y = y_pos[0];
    
    for (int i = 1; i < n; i++) {
        if (x_pos[i] < min_x) min_x = x_pos[i];
        if (x_pos[i] > max_x) max_x = x_pos[i];
        if (y_pos[i] < min_y) min_y = y_pos[i];
        if (y_pos[i] > max_y) max_y = y_pos[i];
    }
    
    std::cout << "Positions updated" << std::endl;
    std::cout << "Placement bounds:" << std::endl;
    std::cout << "  X: [" << std::fixed << std::setprecision(2) 
              << min_x << ", " << max_x << "]" << std::endl;
    std::cout << "  Y: [" << min_y << ", " << max_y << "]" << std::endl;
    
    // Print first few positions for debugging
    std::cout << "First 5 block positions:" << std::endl;
    for (int i = 0; i < std::min(5, n); i++) {
        int block_id = index_to_block[i];
        std::cout << "  Block " << std::setw(3) << block_id 
                  << ": (" << std::setw(8) << std::setprecision(3) << blocks[block_id].x
                  << ", " << std::setw(8) << std::setprecision(3) << blocks[block_id].y
                  << ")" << std::endl;
    }
    
    // Cleanup
    umfpack_di_free_symbolic(&Symbolic);
    umfpack_di_free_numeric(&Numeric);
    delete[] x_pos;
    delete[] y_pos;
    
    std::cout << "Placement solved successfully" << std::endl;
    
    return true;
}
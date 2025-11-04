#ifndef UMFPACK_SOLVER_H
#define UMFPACK_SOLVER_H

#include "weight_matrix.h"
#include <vector>

// Solve the placement linear system using UMFPACK
// Solves: Q * x = b_x  and  Q * y = b_y
// Updates block positions in global blocks map
bool solve_placement(
    const SparseMatrix& Q,
    const std::vector<double>& b_x,
    const std::vector<double>& b_y
);

#endif // UMFPACK_SOLVER_H
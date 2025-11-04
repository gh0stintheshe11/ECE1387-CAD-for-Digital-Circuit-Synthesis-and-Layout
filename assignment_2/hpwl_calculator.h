#ifndef HPWL_CALCULATOR_H
#define HPWL_CALCULATOR_H

// Calculate Half-Perimeter Wirelength for the current placement
// Returns the total HPWL across all nets
double calculate_hpwl();

// Print detailed HPWL statistics
void print_hpwl_stats();

#endif // HPWL_CALCULATOR_H
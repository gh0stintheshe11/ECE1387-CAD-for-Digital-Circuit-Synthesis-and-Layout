#ifndef PARSER_H
#define PARSER_H

#include "circuit.h"
#include <string>

// Parse input file and return Circuit structure
Circuit parse_circuit(const std::string& filename);

// Print circuit summary
void print_circuit_summary(const Circuit& circuit);

#endif // PARSER_H
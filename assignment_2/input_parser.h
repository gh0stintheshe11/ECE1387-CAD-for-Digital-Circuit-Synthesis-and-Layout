#ifndef INPUT_PARSER_H
#define INPUT_PARSER_H

#include <string>

// Parse the input circuit file
bool parse_input_file(const std::string& filename);

// Print circuit information for debugging
void print_circuit_info();

#endif // INPUT_PARSER_H
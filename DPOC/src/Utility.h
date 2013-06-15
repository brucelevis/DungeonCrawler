#ifndef UTILITY_H
#define UTILITY_H

#include <vector>
#include <string>
#include <fstream>

std::vector<std::string> split_string(const std::string& str, char delim);
std::vector<std::string> get_lines(std::ifstream& infile);

#endif

#ifndef UTILITY_H
#define UTILITY_H

#include <vector>
#include <string>
#include <sstream>
#include <fstream>

std::vector<std::string> split_string(const std::string& str, char delim);
std::vector<std::string> get_lines(std::ifstream& infile);

template <typename T>
std::string toString(T val)
{
  std::ostringstream ss;
  ss << val;
  return ss.str();
}

std::string limitString(const std::string& str, int limit);

#endif

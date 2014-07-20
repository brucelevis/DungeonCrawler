#ifndef UTILITY_H
#define UTILITY_H

#include <vector>
#include <string>
#include <sstream>
#include <fstream>

const double PI   = 3.141592653589793238462;
const float  PI_F = 3.14159265358979f;

std::vector<std::string> split_string(const std::string& str, char delim);
std::vector<std::string> get_lines(std::ifstream& infile);
std::vector<std::string> get_lines(std::istringstream& infile);

template <typename T>
std::string toString(T val)
{
  std::ostringstream ss;
  ss << val;
  return ss.str();
}

template <typename T>
T fromString(const std::string& val)
{
  if (val.empty())
    return T();

  T result;
  std::istringstream ss(val);
  ss >> result;
  return result;
}

std::string limit_string(const std::string& str, int limit);

std::string to_lower(const std::string& str);

std::string get_equip_short_name(const std::string& equipName);

std::string get_string_after_first_space(const std::string& str);

bool coinflip();

int random_range(int low, int high);
float rand_float(float low, float high);

char upcase(char c);
std::string capitalize(std::string str);
std::string replace_string(const std::string& str, char from, char to);

float deg2rad(float degs);

bool check_vs_luck(int luck, int luckToBeat);

#endif

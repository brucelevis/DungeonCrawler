#include <sstream>
#include "Utility.h"

std::vector<std::string> split_string(const std::string& str, char delim)
{
  std::istringstream istream(str);

  std::string part;
  std::vector<std::string> elements;
  while (std::getline(istream, part, delim))
  {
    if (part.size() > 0)
      elements.push_back(part);
  }

  return elements;
}

std::vector<std::string> get_lines(std::ifstream& infile)
{
  std::vector<std::string> lines;
  std::string line;
  while (std::getline(infile, line, '\n'))
  {
    lines.push_back(line);
  }
  return lines;
}

std::string limitString(const std::string& str, int limit)
{
  return str.substr(0, limit);
}

#include <sstream>
#include <algorithm>
#include <ctype.h>

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

std::vector<std::string> get_lines(std::istringstream& infile)
{
  std::vector<std::string> lines;
  std::string line;
  while (std::getline(infile, line, '\n'))
  {
    lines.push_back(line);
  }
  return lines;
}

std::string limit_string(const std::string& str, int limit)
{
  return str.substr(0, limit);
}

std::string to_lower(const std::string& str)
{
  std::string cpy = str;

  std::transform(cpy.begin(), cpy.end(), cpy.begin(), ::tolower);

  return cpy;
}

std::string get_equip_short_name(const std::string& equipName)
{
  if (equipName == "Weapon")
  {
    return "Weap";
  }
  else if (equipName == "Shield")
  {
    return "Shld";
  }
  else if (equipName == "Armour")
  {
    return "Armr";
  }
  else if (equipName == "Helmet")
  {
    return "Helm";
  }
  else if (equipName == "Others")
  {
    return "Misc";
  }

  return "";
}

std::string get_string_after_first_space(const std::string& str)
{
  std::string buff;

  bool foundSpace = false;
  bool foundNonSpace = false;

  for (size_t i = 0; i < str.size(); i++)
  {
    if (!foundSpace && str[i] == ' ')
    {
      foundSpace = true;
    }
    else if (foundSpace && (str[i] != ' ' || foundNonSpace))
    {
      foundNonSpace = true;
      buff += str[i];
    }
  }

  return buff;
}

bool coinflip()
{
  return (bool)(rand()%2);
}

int random_range(int low, int high)
{
  return low + ((rand()%high) - low);
}

float rand_float(float low, float high)
{
  return low + (float)rand()/((float)RAND_MAX/(high));
}

char upcase(char c)
{
  if (c >= 'a' && c <= 'z')
    return c - 32;

  return c;
}

std::string capitalize(std::string str)
{
  str[0] = upcase(str[0]);
  return str;
}

std::string replace_string(const std::string& str, char from, char to)
{
  std::string transform = str;
  std::transform(transform.begin(), transform.end(), transform.begin(),
      [=](char c) { return from == c ? to : c; });

  return transform;
}

#ifndef GAMELIB_GAMELIB_STRINGS_H
#define GAMELIB_GAMELIB_STRINGS_H

#include <string>
#include <sstream>

namespace gamelib
{
  namespace str
  {
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

    template <typename Container>
    Container split_string(const std::string& str, char delim)
    {
      std::istringstream istream(str);

      std::string part;
      Container elements;
      while (std::getline(istream, part, delim))
      {
        elements.push_back(part);
      }

      return elements;
    }

    template <typename T>
    std::string toString(const T& val)
    {
      std::ostringstream stream;
      stream << val;
      return stream.str();
    }
  }
}

#endif

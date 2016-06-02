#ifndef GAMELIB_IO_H
#define GAMELIB_IO_H

#include <string>

namespace gamelib
{
  namespace io
  {
    std::string loadFile(const std::string& filename);

    std::string getCurrentWorkingDirectory();
  }
}

#endif

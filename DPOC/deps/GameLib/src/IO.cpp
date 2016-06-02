#include <stdexcept>
#include <cstdlib>
#include <fstream>

#ifdef _WIN32
# include <direct.h>
# define getcwd _getcwd
#else
# include <unistd.h>
#endif

#include <GameLib/IO.h>

namespace gamelib
{
  namespace io
  {
    std::string loadFile(const std::string& filename)
    {
      std::string buffer;
      std::ifstream ifile(filename.c_str());

      if (ifile.is_open())
      {
        while (!ifile.eof())
        {
          char c = ifile.get();
          buffer += c;
        }

        ifile.close();
      }
      else
      {
        throw std::runtime_error("Unable to open file: " + filename);
      }

      return buffer;
    }

    std::string getCurrentWorkingDirectory()
    {
      static char current_path[FILENAME_MAX];
      getcwd(current_path, sizeof(current_path));
      return current_path;
    }
  }
}

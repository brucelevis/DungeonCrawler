#ifdef _WIN32
# include <windows.h>
#else
# include <cstdio>
#endif

#include <cstdlib>
#include <cstdarg>

#include "logger.h"
#include "Error.h"

namespace
{
  void show_message_box(const std::string& message)
  {
#ifdef _WIN32
    MessageBox(nullptr, message.c_str(), "Error", MB_ICONERROR | MB_OK);
#else
    printf("%s\n", message.c_str());
#endif
  }
}

void error_and_die(const char* fmt, ...)
{
  char buffer[512];

  va_list args;
  va_start(args, fmt);

  vsprintf(buffer, fmt, args);

  va_end(args);

  show_message_box(buffer);
  exit(1);
}

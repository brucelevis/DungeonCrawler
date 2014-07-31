#ifndef LOGGER_H
#define LOGGER_H

#include <cstdio>
#include <ctime>
#include <cstring>

class Console;

class Logger
{
public:
  static Logger& instance();
  ~Logger();

  void trace(const char* file, int line, const char* fmt, ...);
  void setConsole(Console* console);
private:
  Logger();
private:
  FILE* m_logFile;
  Console* m_console;
};

#define START_LOG Logger::instance();
#define TRACE(...) Logger::instance().trace(__FILE__, __LINE__, __VA_ARGS__)

#endif

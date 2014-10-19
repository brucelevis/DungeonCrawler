#include <cstdarg>
#include <sstream>

#include "Console.h"
#include "logger.h"

std::string getTimeStamp()
{
  char timestr[32];

  time_t _time;
  time(&_time);

  strncpy(timestr, ctime(&_time), 32);
  timestr[strlen(timestr) - 1] = '\0';

  return timestr;
}

Logger& Logger::instance()
{
  static Logger logger;
  return logger;
}

Logger::Logger()
: m_logFile(0),
  m_console(0)
{
  m_logFile = fopen("log.txt", "w");
  fclose(m_logFile);
}

Logger::~Logger()
{
}

void Logger::trace(const char* file, int line, const char* fmt, ...)
{
  char buffer[1024];
  memset(buffer, '\0', 1024);

  va_list args;
  va_start(args, fmt);

  //vsprintf(buffer, fmt, args);
  vsnprintf(buffer, 1024, fmt, args);

  va_end(args);

  std::ostringstream stream;
  stream << "[" << getTimeStamp() << "] " << file << ":" << line << ": " << buffer << "\n";

  m_logFile = fopen("log.txt", "a");
  fprintf(m_logFile, "%s", stream.str().c_str());
  fclose(m_logFile);

  if (m_console)
  {
    m_console->add(buffer);
  }
}

void Logger::setConsole(Console* console)
{
  m_console = console;
}

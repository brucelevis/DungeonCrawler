#ifndef ERROR_H_
#define ERROR_H_

#include <string>
#include "logger.h"

void error_and_die(const char* fmt, ...);

#define CRASH(...) \
  do { \
    Logger::instance().trace(__FILE__, __LINE__, __VA_ARGS__); \
    error_and_die(__VA_ARGS__); \
  } while (0)

#endif /* ERROR_H_ */

#ifndef LOGGER_H
#define LOGGER_H

#include <stdio.h>
#include <time.h>
#include <string.h>

#define START_LOG \
do { \
    fclose(fopen("log.txt", "w")); \
} while (0)

#define TRACE(...) \
do { \
    char timestr[32]; \
    FILE* log = fopen("log.txt", "a"); \
    time_t _time; \
    time(&_time); \
    strncpy(timestr, ctime(&_time), 32); \
    timestr[strlen(timestr) - 1] = '\0'; \
    fprintf(log, "[%s] %s:%d: ", timestr, __FILE__, __LINE__); \
    fprintf(log, __VA_ARGS__); \
    fprintf(log, "\n"); \
    fclose(log); \
} while(0)

#endif

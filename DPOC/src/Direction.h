#ifndef DIRECTION_H
#define DIRECTION_H

#include <string>

enum Direction
{
  DIR_RIGHT = 0,
  DIR_LEFT = 1,
  DIR_UP = 2,
  DIR_DOWN = 3,
  DIR_RANDOM = 4
};

Direction directionFromString(const std::string& dirStr);
std::string directionToString(Direction dir);

#endif

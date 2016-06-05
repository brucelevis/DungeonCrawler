#include "logger.h"
#include "Direction.h"

Direction directionFromString(const std::string& dirStr)
{
  if (dirStr == "DIR_RIGHT") return DIR_RIGHT;
  if (dirStr == "DIR_LEFT") return DIR_LEFT;
  if (dirStr == "DIR_DOWN") return DIR_DOWN;
  if (dirStr == "DIR_UP") return DIR_UP;
  if (dirStr == "DIR_RANDOM") return DIR_RANDOM;

  TRACE("Error: Unknown direction %s!", dirStr.c_str());

  return DIR_RANDOM;
}

std::string directionToString(Direction dir)
{
  if (dir == DIR_RIGHT) return "DIR_RIGHT";
  if (dir == DIR_LEFT) return "DIR_LEFT";
  if (dir == DIR_DOWN) return "DIR_DOWN";
  if (dir == DIR_UP) return "DIR_UP";
  if (dir == DIR_RANDOM) return "DIR_RANDOM";

  TRACE("Error: Unknown direction %d!", dir);

  return "DIR_RANDOM";
}

int getAngleFromDirection(Direction dir)
{
  switch (dir)
  {
  case DIR_RIGHT:
    return 0;
  case DIR_LEFT:
    return 180;
  case DIR_UP:
    return 90;
  case DIR_DOWN:
    return 270;
  default:
    return 0;
  }
}

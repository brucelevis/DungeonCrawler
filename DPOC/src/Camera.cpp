#include <sstream>
#include <cmath>

#include "Camera.h"

void Camera::rotate(float rads)
{
  Vec2 _dir = dir;
  Vec2 _plane = plane;

  dir.x = _dir.x * cos(rads) - _dir.y * sin(rads);
  dir.y = _dir.x * sin(rads) + _dir.y * cos(rads);
						
  plane.x = _plane.x * cos(rads) - _plane.y * sin(rads);
  plane.y = _plane.x * sin(rads) + _plane.y * cos(rads);  
}

std::string Camera::dump() const
{
  std::ostringstream ss;
  ss << "[ pos={" << pos.x << ", " << pos.y << "} dir={" << dir.x << ", " << dir.y << "} plane={" << plane.x << ", " << plane.y << "} ]\n";
  return ss.str();
}

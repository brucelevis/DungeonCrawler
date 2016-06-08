#ifndef CAMERA_H
#define CAMERA_H

#include <string>

#include "Vec2.h"

struct Camera 
{
	Camera(Vec2 _pos, Vec2 _dir, Vec2 _plane)
    : pos(_pos), dir(_dir), plane(_plane)
  {}

  void rotate(float rads);

  std::string dump() const;

  Vec2 pos, dir, plane;

  Camera clone()
  {
    return Camera{pos, dir, plane};
  }
};

#endif

#ifndef VEC_2
#define VEC_2

struct Vec2 
{
  Vec2() : x(0), y(0) {}
  Vec2(float _x, float _y) : x(_x), y(_y) {}
  Vec2(const Vec2& rhs) : x(rhs.x), y(rhs.y) {}
  Vec2& operator=(const Vec2& rhs) {
    if (this != &rhs)
    {
      x = rhs.x;
      y = rhs.y;
    }
    return *this;
  }

  float x, y;
};

#endif

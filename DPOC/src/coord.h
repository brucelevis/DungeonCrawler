#ifndef COORD_H
#define COORD_H

typedef struct coord_t
{
  int x, y;
} coord_t;

static inline bool coord_valid(const coord_t& coord)
{
  return coord.x != -1 && coord.y != -1;
}

#endif

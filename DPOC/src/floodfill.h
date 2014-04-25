#ifndef FLOODFILL_H
#define FLOODFILL_H

#include <queue>
#include "coord.h"

/**
 * Floodfills an array with the specified value.
 * @param map 	Flattened 2D array.
 *	@param map_w	Columns in the array.
 * @param map_h	Rows in the array.
 * @param x	X origin of floodfill.
 * @param y	Y origin of floodfill-
 * @param value	Value to fill with.
 * @return The number of tiles filled.
 */
template<typename T>
int floodfill(T* map, int map_w, int map_h, int x, int y, T value)
{
  int sum = 1;
  int index = y * map_w + x;
  T src_value = map[index];

  std::queue<coord_t> flood_queue;

  flood_queue.push(coord_t());

  flood_queue.front().x = x;
  flood_queue.front().y = y;

  map[index] = value;

  while (!flood_queue.empty())
  {
    int px, py;
    coord_t coord = flood_queue.front();

    flood_queue.pop();

    for (py = coord.y - 1; py <= coord.y + 1; py++)
    {
      for (px = coord.x - 1; px <= coord.x + 1; px++)
      {

        if (px == coord.x && py == coord.y)
          continue;

        // Ignore diagonals.
        if (px == coord.x || py == coord.y)
        {
          index = py * map_w + px;

          if (map[index] == src_value)
          {
            map[index] = value;
            sum++;

            coord_t new_coord =
            { px, py };
            flood_queue.push(new_coord);
          }
        }

      }
    }

  }

  return sum;
}

#endif

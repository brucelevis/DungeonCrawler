#ifndef MAP_H
#define MAP_H

#include <vector>

#include <SFML/Graphics.hpp>

#include "Config.h"
#include "Entity.h"

struct Tile
{
  int tileX, tileY;
  int zone;
};

struct Warp
{
  int srcX, srcY;
  int dstX, dstY;
  std::string destMap;
};

class Map
{
  friend class Editor;
public:
  ~Map();
  int getWidth() const { return m_width; }
  int getHeight() const { return m_height; }

  void update();
  void draw(sf::RenderTarget& target, const coord_t& view);

  Tile* getTileAt(int x, int y, int layer);

  bool saveToFile(const std::string& filename) const;

  static Map* loadFromFile(const std::string& filename);
private:
  Map();
  Map(const Map&);
  Map& operator=(const Map&);

  int getNumberOfTiles() const
  {
    return m_width * m_height;
  }
private:
  Tile* m_tiles[config::MAX_LAYERS];
  int m_width, m_height;
  std::vector<Entity*> m_entities;
  std::string m_music;
  std::vector<Warp> m_warps;

  sf::Texture* m_tileset;
};

#endif

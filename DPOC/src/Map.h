#ifndef MAP_H
#define MAP_H

#include <vector>
#include <string>
#include <map>

#include <SFML/Graphics.hpp>

#include <BGL/coord.h>
#include "Config.h"
#include "Entity.h"

struct Tile
{
  int tileX, tileY;
  int zone;
  bool solid;
};

struct Warp
{
  int srcX, srcY;
  int dstX, dstY;
  std::string destMap;
};

static inline std::string getWarpTargetName(const Warp& warp)
{
  return "Resources/Maps/" + warp.destMap;
}

class Map
{
  friend class Editor;
public:
  ~Map();
  int getWidth() const { return m_width; }
  int getHeight() const { return m_height; }

  void update();
  void draw(sf::RenderTarget& target, const bgl::coord_t& view);

  Tile* getTileAt(int x, int y, int layer);
  bool warpAt(int x, int y) const;
  const Warp* getWarpAt(int x, int y) const;

  std::string getName() const { return m_name; }
  std::string getMusic() const { return m_music; }

  bool saveToFile(const std::string& filename);
  static Map* loadFromFile(const std::string& filename);
  static Map* loadTiledFile(const std::string& filename);

  const std::vector<Entity*>& getEntities() const { return m_entities; }

  bool blocking(int x, int y);
  bool inside(int x, int y) const;

  std::vector<std::string> checkEncounter(int x, int y);

  std::string xmlDump() const;

  std::string getBattleBackground() const { return m_battleBackground; }
private:
  Map();
  Map(const Map&);
  Map& operator=(const Map&);

  int getNumberOfTiles() const
  {
    return m_width * m_height;
  }
private:
  std::vector<Tile*> m_tiles;
  int m_width, m_height;
  std::vector<Entity*> m_entities;
  std::string m_music;
  std::vector<Warp> m_warps;

  int m_encounterRate;
  std::multimap<int, std::vector<std::string> > m_encounters;

  sf::Texture* m_tileset;
  std::string m_name;

  std::string m_battleBackground;
};

#endif

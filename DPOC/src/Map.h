#ifndef MAP_H
#define MAP_H

#include <vector>
#include <string>
#include <map>
#include <unordered_map>
#include <set>

#include <SFML/Graphics.hpp>

#include "Direction.h"
#include "coord.h"
#include "Config.h"
#include "Entity.h"
#include "Trap.h"

class Encounter;

struct Tile
{
  int tileX, tileY;
  bool solid;
  int tileId;
  float height;
};

struct Warp
{
  int srcX, srcY;
  int dstX, dstY;
  std::string destMap;
  Direction dir;
};

static inline std::string getWarpTargetName(const Warp& warp)
{
  return "Maps/" + warp.destMap;
}

class Map
{
  friend class Editor;
public:
  ~Map();
  int getWidth() const { return m_width; }
  int getHeight() const { return m_height; }

  void update();

  void setTileAt(int x, int y, const std::string& layer, int tileId);
  Tile* getTileAt(int x, int y, const std::string& layer);
  bool warpAt(int x, int y) const;
  const Warp* getWarpAt(int x, int y) const;

  bool trapAt(int x, int y) const;
  const Trap* getTrapAt(int x, int y) const;
  void disableTrap(const Trap* trap);

  std::string getName() const { return m_name; }
  std::string getMusic() const { return m_music; }

  static Map* loadTiledFile(const std::string& filename);
  static Map* createEmptyFrom(const Map* other, int width, int height);

  const std::vector<Entity*>& getEntities() const { return m_entities; }

  bool blocking(int x, int y);
  bool inside(int x, int y) const;

  const Encounter* checkEncounter();

  std::string xmlDump() const;

  std::vector<sf::Image> getTilesetImages() const;
  std::string getBattleBackground() const { return m_battleBackground; }

  const sf::Texture* getBackground() const { return m_background; }

  void explore(int x, int y);
  bool isExplored(int x, int y) const;

  static void updateExplored(const std::string& mapName, const std::vector<bool>& explored);
private:
  Map();
  Map(const Map&);
  Map& operator=(const Map&);

  int getNumberOfTiles() const
  {
    return m_width * m_height;
  }

  std::string getTrapKey(const Trap* trap) const;
private:
  std::map<std::string, Tile*> m_tiles;
  int m_width, m_height;
  std::vector<Entity*> m_entities;
  std::string m_music;
  std::vector<Warp> m_warps;
  std::vector<Trap> m_traps;

  int m_encounterRate;
  std::vector< std::string > m_encounters;

  std::string m_tilesetName;
  sf::Texture* m_tileset;
  std::string m_name;

  std::string m_battleBackground;
  sf::Texture* m_background;

  static std::unordered_map<std::string, std::vector<bool>> s_explored;
};

#endif

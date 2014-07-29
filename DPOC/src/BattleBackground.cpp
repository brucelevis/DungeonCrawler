#include "Raycaster.h"

#include "Map.h"
#include "Config.h"

#include "BattleBackground.h"

namespace
{
  static const int MAP_SIZE = 7;

  Tile* findWallTile(Map* map, const int x, const int y, Direction dir)
  {
    int xInc = dir == DIR_LEFT ? -1 : dir == DIR_RIGHT ? 1 : 0;
    int yInc = dir == DIR_UP   ? -1 : dir == DIR_DOWN  ? 1 : 0;

    if (xInc != 0)
    {
      int px = x;

      while (px >= 0 && px < map->getWidth())
      {
        Tile* wallTile = map->getTileAt(px, y, "wall");
        if (wallTile && wallTile->tileId >= 0)
        {
          return wallTile;
        }
        px += xInc;
      }
    }

    if (yInc != 0)
    {
      int py = y;
      while (py >= 0 && py < map->getHeight())
      {
        Tile* wallTile = map->getTileAt(x, py, "wall");
        if (wallTile && wallTile->tileId >= 0)
        {
          return wallTile;
        }
        py += yInc;
      }
    }

    return 0;
  }
}

BattleBackground::BattleBackground(Map* sourceMap, int x, int y, Direction playerDir)
 : m_raycaster(new Raycaster(config::RAYCASTER_RES_X, config::RAYCASTER_RES_Y)),
   m_currentMap(Map::createEmptyFrom(sourceMap, MAP_SIZE, MAP_SIZE)),
   m_camera(Vec2((float)MAP_SIZE / 2, (float)MAP_SIZE / 2), Vec2(-1, 0), Vec2(0, -0.66f))
{
  m_raycaster->setTilemap(m_currentMap);

  Tile* wallTile  = findWallTile(sourceMap, x, y, playerDir);
  Tile* floorTile = sourceMap->getTileAt(x, y, "floor");
  Tile* ceilTile  = sourceMap->getTileAt(x, y, "ceiling");
  for (int py = 0; py < m_currentMap->getHeight(); py++)
  {
    for (int px = 0; px < m_currentMap->getWidth(); px++)
    {
      m_currentMap->setTileAt(px, py, "floor", floorTile->tileId);
      m_currentMap->setTileAt(px, py, "ceiling", ceilTile->tileId);
    }
  }

  for (int px = 0; px < m_currentMap->getWidth(); px++)
  {
    int h = m_currentMap->getHeight() - 1;

    m_currentMap->setTileAt(px, 0, "wall", wallTile->tileId);
    m_currentMap->setTileAt(px, h, "wall", wallTile->tileId);
  }
  for (int py = 0; py < m_currentMap->getHeight(); py++)
  {
    int w = m_currentMap->getWidth() - 1;

    m_currentMap->setTileAt(0, py, "wall", wallTile->tileId);
    m_currentMap->setTileAt(w, py, "wall", wallTile->tileId);
  }
}

BattleBackground::~BattleBackground()
{
  delete m_raycaster;
  delete m_currentMap;
}

const sf::Texture& BattleBackground::getBackgroundTexture()
{
  m_raycasterBuffer.create(config::RAYCASTER_RES_X, config::RAYCASTER_RES_Y, sf::Color::Black);
  m_raycaster->raycast(&m_camera, m_raycasterBuffer);

  m_raycasterTexture.loadFromImage(m_raycasterBuffer);

  return m_raycasterTexture;
}

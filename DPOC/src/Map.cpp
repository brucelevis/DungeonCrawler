#include <fstream>

#include "Cache.h"
#include "logger.h"
#include "Map.h"

Map::Map()
 : m_width(0),
   m_height(0)
{
  for (int i = 0; i < config::MAX_LAYERS; i++)
  {
    m_tiles[i] = 0;
  }

  m_tileset = cache::loadTexture("Resources/DqTileset.png");
}

Map::~Map()
{
  for (int i = 0; i < config::MAX_LAYERS; i++)
  {
    if (m_tiles[i])
      delete[] m_tiles[i];
  }

  for (auto it = m_entities.begin(); it != m_entities.end(); ++it)
    delete *it;

  cache::releaseTexture("Resources/DqTileset.png");
}

void Map::update()
{
  for (auto it = m_entities.begin(); it != m_entities.end(); ++it)
    (*it)->update();
}

void Map::draw(sf::RenderTarget& target, const coord_t& view)
{
  sf::Sprite sprite;
  sprite.setTexture(*m_tileset);

  for (int y = 0; y < m_height; y++)
  {
    for (int x = 0; x < m_width; x++)
    {
      for (int layer = 0; layer < config::MAX_LAYERS; layer++)
      {
        Tile* tile = getTileAt(x, y, layer);

        sprite.setTextureRect(sf::IntRect(tile->tileX * config::TILE_W, tile->tileY * config::TILE_H, config::TILE_W, config::TILE_H));
        sprite.setPosition(x * config::TILE_W - view.x, y * config::TILE_H - view.y);
        target.draw(sprite);
      }
    }
  }

  for (auto it = m_entities.begin(); it != m_entities.end(); ++it)
  {
    (*it)->draw(target, view);
  }

//  for (auto it = m_warps.begin(); it != m_warps.end(); ++it)
//  {
//    sf::RectangleShape rect;
//    rect.setSize(sf::Vector2f(config::TILE_W, config::TILE_H));
//    rect.setFillColor(sf::Color::Red);
//    rect.setPosition(it->srcX*config::TILE_W - view.x, it->srcY*config::TILE_H - view.y);
//    target.draw(rect);
//  }
}

bool Map::saveToFile(const std::string& filename)
{
  bool success = false;

  TRACE("Saving map to %s", filename.c_str());

  std::ofstream ofile(filename.c_str());
  if (ofile.is_open())
  {
    m_name = filename;

    ofile << m_width << " " << m_height << " " << config::MAX_LAYERS << " ";
    for (int i = 0; i < config::MAX_LAYERS; i++)
    {
      for (int j = 0; j < m_width * m_height; j++)
      {
        ofile << m_tiles[i][j].tileX << " "
              << m_tiles[i][j].tileY << " "
              << m_tiles[i][j].zone << " ";
      }
    }
    ofile << m_music << " ";

    ofile << m_entities.size() << " ";
    for (auto it = m_entities.begin(); it != m_entities.end(); ++it)
    {
      ofile << (*it)->getName() << " "
            << (int)(*it)->x << " "
            << (int)(*it)->y << " ";
    }

    ofile << m_warps.size() << " ";
    for (auto it = m_warps.begin(); it != m_warps.end(); ++it)
    {
      ofile << it->srcX << " "
            << it->srcY << " "
            << it->dstX << " "
            << it->dstY << " "
            << it->destMap << " ";
    }

    ofile.close();

    success = true;
  }
  else
  {
    TRACE("Unable to open %s for writing.", filename.c_str());
  }

  return success;
}

Map* Map::loadFromFile(const std::string& filename)
{
  TRACE("Loading map from %s", filename.c_str());

  std::ifstream ifile(filename.c_str());

  if (ifile.is_open())
  {
    Map* map = new Map;
    map->m_name = filename;

    int layers;

    ifile >> map->m_width >> map->m_height;
    ifile >> layers;

    for (int i = 0; i < layers; i++)
    {
      map->m_tiles[i] = new Tile[map->m_width * map->m_height]();
      for (int j = 0; j < map->m_width * map->m_height; j++)
      {
        Tile tile;
        ifile >> tile.tileX >> tile.tileY >> tile.zone;

        map->m_tiles[i][j] = tile;
      }
    }

    ifile >> map->m_music;

    int numEntities;
    ifile >> numEntities;

    for (int i = 0; i < numEntities; i++)
    {
      std::string entName;
      int entX, entY;

      ifile >> entName >> entX >> entY;

      Entity* entity = new Entity(entName);
      entity->x = entX;
      entity->y = entY;

      map->m_entities.push_back(entity);
    }

    int numWarps;
    ifile >> numWarps;
    for (int i = 0; i < numWarps; i++)
    {
      Warp warp;
      ifile >> warp.srcX
            >> warp.srcY
            >> warp.dstX
            >> warp.dstY
            >> warp.destMap;
      map->m_warps.push_back(warp);
    }

    ifile.close();

    return map;
  }
  else
  {
    TRACE("Unable to open file %s for reading", filename.c_str());
  }

  return 0;
}

Tile* Map::getTileAt(int x, int y, int layer)
{
  if (x < 0 || y < 0 || x >= m_width || y >= m_height)
    return 0;

  int index = y * m_width + x;
  if (index >= 0 && index < getNumberOfTiles())
  {
    return &m_tiles[layer][index];
  }
  return 0;
}

bool Map::warpAt(int x, int y) const
{
  for (auto it = m_warps.begin(); it != m_warps.end(); ++it)
  {
    if (it->srcX == x && it->srcY == y)
    {
      return true;
    }
  }
  return false;
}

const Warp* Map::getWarpAt(int x, int y) const
{
  for (auto it = m_warps.begin(); it != m_warps.end(); ++it)
  {
    if (it->srcX == x && it->srcY == y)
    {
      return &(*it);
    }
  }
  return 0;
}

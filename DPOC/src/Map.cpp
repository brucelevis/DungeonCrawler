#include <fstream>

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
}

bool Map::saveToFile(const std::string& filename) const
{
  bool success = false;

  TRACE("Saving map to %s", filename.c_str());

  std::ofstream ofile(filename.c_str());
  if (ofile.is_open())
  {
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

    ifile.close();

    return map;
  }
  else
  {
    TRACE("Unable to open file %s for reading", filename.c_str());
  }

  return 0;
}

#include <sstream>
#include <fstream>
#include <set>

#include "Chest.h"
#include "Door.h"
#include "Persistent.h"
#include "Config.h"
#include "Direction.h"
#include "Utility.h"
#include "TiledLoader.h"
#include "Cache.h"
#include "logger.h"
#include "Encounter.h"

#include "Map.h"

namespace
{
  void _parse_trap(const std::string& trapString, std::string& trapType, int& luckToBeat)
  {
    std::vector<std::string> trapData = split_string(trapString, ',');
    trapType = trapData[0];
    luckToBeat = fromString<int>(trapData[1]);
  }

  std::unordered_map<std::string, std::string> _parse_script_arguments(const TiledLoader& loader, size_t objectIndex)
  {
    std::unordered_map<std::string, std::string> scriptArguments;

    for (const auto& pair : loader.getObject(objectIndex)->properties)
    {
      if (pair.first.front() == '@')
      {
        scriptArguments[pair.first] = pair.second;
      }
    }

    return scriptArguments;
  }
}

std::unordered_map<std::string, std::vector<bool>> Map::s_explored;

Map::Map()
 : m_width(0),
   m_height(0),
   m_encounterRate(0),
   m_tileset(0),
   m_background(0)
{
}

Map::~Map()
{
  for (auto it = m_tiles.begin(); it != m_tiles.end(); ++it)
  {
    delete[] it->second;
  }

  for (auto it = m_entities.begin(); it != m_entities.end(); ++it)
    delete *it;

  cache::releaseTexture(m_tileset);
  cache::releaseTexture(m_background);
}

void Map::update()
{
  for (auto it = m_entities.begin(); it != m_entities.end(); ++it)
    (*it)->update();
}

Map* Map::loadTiledFile(const std::string& filename)
{
  Map* map = 0;

  TRACE("Loading map %s", filename.c_str());

  TiledLoader loader;
  if (loader.loadFromFile(config::res_path(filename)))
  {
    map = new Map;
    map->m_name = filename;
    map->m_music = loader.getProperty("music");

    // The tile size is used everywhere so store it in a global.
    config::TILE_W = loader.getTileWidth();
    config::TILE_H = loader.getTileHeight();

    map->m_encounterRate = 30;
    if (!loader.getProperty("encounterRate").empty())
    {
      map->m_encounterRate = fromString<int>(loader.getProperty("encounterRate"));
    }

    map->m_battleBackground = loader.getProperty("battleBackground");

    std::string bg = loader.getProperty("background");
    if (bg.size())
    {
      map->m_background = cache::loadTexture("Backgrounds/" + bg);
    }

    TRACE("Map: Loading tilesets");

    std::vector<std::string> tilesets = loader.getTilesets();
    for (auto it = tilesets.begin(); it != tilesets.end(); ++it)
    {
      const TiledLoader::Tileset* tileset = loader.getTileset(*it);
      if (tileset->startTileIndex == 1)
      {
        TRACE("Map: loading tileset %s", config::res_path("Maps/" + tileset->tilesetSource).c_str());

        map->m_tilesetName = "Maps/" + tileset->tilesetSource;
        map->m_tileset = cache::loadTexture("Maps/" + tileset->tilesetSource);

        if (!map->m_tileset)
        {
          TRACE("Map: Unable to load tileset");

          delete map;
          return 0;
        }
      }
    }

    TRACE("Map: Loading layers");

    std::vector<std::string> layers = loader.getLayers();
    for (size_t i = 0; i < layers.size(); i++)
    {
      // Blocking layer is special.
      if (to_lower(layers[i]) == "blocking")
        continue;

      std::string layerName = to_lower(layers[i]);

      const TiledLoader::Layer* layer = loader.getLayer(layers[i]);

      map->m_width = layer->width;
      map->m_height = layer->height;

      map->m_tiles[layerName] = new Tile[map->m_width * map->m_height]();

      for (size_t j = 0; j < layer->tiles.size(); j++)
      {
        int tileId = layer->tiles[j];

        Tile tile;
        tile.solid = false;
        tile.tileX = 0;
        tile.tileY = 0;

        if (tileId > 0)
        {
          tile.tileX = (tileId - 1) % (map->m_tileset->getSize().x / config::TILE_W);
          tile.tileY = (tileId - 1) / (map->m_tileset->getSize().x / config::TILE_H);
        }
        tile.tileId = tileId - 1; // -1 then means no tile.

        map->m_tiles[layerName][j] = tile;
      }
    }

    // Blocking layer
    for (auto it = layers.begin(); it != layers.end(); ++it)
    {
      if (to_lower(*it) == "blocking")
      {
        const TiledLoader::Layer* layer = loader.getLayer(*it);

        for (size_t i = 0; i < layer->tiles.size(); i++)
        {
          int tileId = layer->tiles[i];
          if (tileId != 0)
            map->m_tiles["floor"][i].solid = true;
        }
      }
    }

    for (size_t objectIndex = 0; objectIndex < loader.getNumberOfObjects(); objectIndex++)
    {
      const TiledLoader::Object* object = loader.getObject(objectIndex);
      if (object->tileId > 0)
      {
        const TiledLoader::Tileset* tileset = loader.findTilesetMatchingTileIndex(object->tileId);
        int tileId = object->tileId - tileset->startTileIndex;

        if (tileset)
        {
          std::string spriteSheet = "Maps/" + tileset->tilesetSource;
          sf::Texture* texture = cache::loadTexture(spriteSheet);

          std::string name = object->name;
          float walkSpeed = fromString<float>(loader.getObjectProperty(objectIndex, "walkSpeed"));
          std::string walkThroughProperty = loader.getObjectProperty(objectIndex, "walkThrough");

          int objX = object->x / config::TILE_W;
          int objY = object->y / config::TILE_H;

          int tileX = tileId % (texture->getSize().x / config::TILE_W);
          int tileY = tileId / (texture->getSize().x / config::TILE_H);

          if (name.empty())
          {
            name = "anonymous_object@[" + toString(objX) + "," + toString(objY) + "]";
          }

          TRACE("New object: tileset=%s, tileId=%d, name=%s, x=%d, y=%d",
              tileset->tilesetSource.c_str(), tileId, name.c_str(), objX, objY);

          Entity* entity = 0;

          if (loader.getObjectType(objectIndex) == "chest")
          {
            std::string items = loader.getObjectProperty(objectIndex, "items");
            std::string trap = loader.getObjectProperty(objectIndex, "trap");

            if (trap.empty())
            {
              entity = new Chest(name, split_string(items, ','));
            }
            else
            {
              std::string trapType;
              int luckToBeat;

              _parse_trap(trap, trapType, luckToBeat);

              entity = new Chest(name, split_string(items, ','), trapType, luckToBeat);
            }
          }
          else if (loader.getObjectType(objectIndex) == "door")
          {
            std::string keyRequired = loader.getObjectProperty(objectIndex, "key");
            std::string trap = loader.getObjectProperty(objectIndex, "trap");
            std::string doorType = loader.getObjectProperty(objectIndex, "type");
            Door::DoorType type = doorType == "2" ? Door::Door_TwoWay : Door::Door_OneWay;

            if (trap.empty())
            {
              entity = new Door(name, keyRequired, type);
            }
            else
            {
              std::string trapType;
              int luckToBeat;

              _parse_trap(trap, trapType, luckToBeat);

              entity = new Door(name, keyRequired, trapType, luckToBeat, type);
            }
          }
          else
          {
            entity = new Entity(name);
          }

          entity->setPosition(objX, objY);
          entity->setTag(name + "@@" + map->m_name);
          entity->setWalkSpeed(walkSpeed);
          entity->setType(loader.getObjectType(objectIndex));

          if (walkThroughProperty == "true")
          {
            entity->setWalkThrough(true);
          }

          entity->loadScripts(
              loader.getObjectProperty(objectIndex, "talkScript"),
              loader.getObjectProperty(objectIndex, "stepScript"),
              _parse_script_arguments(loader, objectIndex));

          TileSprite* tileSprite = new TileSprite(texture, tileX, tileY);
          entity->setSprite(tileSprite);
          map->m_entities.push_back(entity);

          // cache::releaseTexture(texture);
        }
      }
      else
      {
        std::string name = object->name;
        if (to_lower(name) == "warp")
        {
          std::string dirStr = loader.getObjectProperty(objectIndex, "direction");

          Warp warp;
          warp.srcX = object->x / config::TILE_W;
          warp.srcY = object->y / config::TILE_H;
          warp.dstX = fromString<int>(loader.getObjectProperty(objectIndex, "destX"));
          warp.dstY = fromString<int>(loader.getObjectProperty(objectIndex, "destY"));
          warp.destMap = loader.getObjectProperty(objectIndex, "destMap");
          warp.dir = dirStr.empty() ? DIR_RANDOM : directionFromString(dirStr);
          map->m_warps.push_back(warp);

          TRACE("New warp: srcX=%d, srcY=%d, dstX=%d, dstY=%d, dstMap=%s, direction=%s",
              warp.srcX, warp.srcY, warp.dstX, warp.dstY, warp.destMap.c_str(), dirStr.c_str());
        }
        else if (to_lower(name) == "trap")
        {
          int trapX = object->x / config::TILE_W;
          int trapY = object->y / config::TILE_H;

          std::string trapType = loader.getObjectType(objectIndex);
          int luckToBeat = fromString<int>(loader.getObjectProperty(objectIndex, "luck"));

          map->m_traps.push_back( Trap { trapType, luckToBeat, trapX, trapY } );

          TRACE("New trap: trapX=%d, trapY=%d, trapType=%s, luck=%d",
              trapX, trapY, trapType.c_str(), luckToBeat);
        }
        else
        {
          // Entities without picture.
          std::string name = object->name;

          int objX = object->x / config::TILE_W;
          int objY = object->y / config::TILE_H;

          if (name.empty())
          {
            name = "anonymous_object@[" + toString(objX) + "," + toString(objY) + "]";
          }

          Entity* entity = new Entity(name);
          entity->setPosition(objX, objY);
          entity->setTag(name + "@@" + map->m_name);
          entity->setWalkSpeed(0);
          entity->setWalkThrough(true);
          entity->setIsVisible(false);
          entity->setType(loader.getObjectType(objectIndex));

          entity->loadScripts(
              loader.getObjectProperty(objectIndex, "talkScript"),
              loader.getObjectProperty(objectIndex, "stepScript"),
              _parse_script_arguments(loader, objectIndex));

          map->m_entities.push_back(entity);
        }
      }
    }

    TRACE("Reading encounters");
    std::string encounters_str = loader.getProperty("encounters");
    std::vector<std::string> encounters = split_string(encounters_str, ',');
    map->m_encounters = encounters;

    TRACE(" -> %s", encounters_str.c_str());

    TRACE("Updating explored vector for map");

    // Initiate explored vector for this map, unless previously done.
    if (s_explored.count(map->getName()) == 0)
    {
      s_explored[map->getName()] = std::vector<bool>(map->m_width * map->m_height, false);
    }
    else
    {
      // If size has changed the map has changed in some way, so reset it.
      size_t size = s_explored[map->getName()].size();
      if ((int)size != map->getWidth() * map->getHeight())
      {
        s_explored[map->getName()] = std::vector<bool>(map->m_width * map->m_height, false);
      }
    }

    TRACE("Map loading completed!");
  }
  else
  {
    TRACE("Unable to open tiled file %s", filename.c_str());
  }

  return map;
}

Map* Map::createEmptyFrom(const Map* other, int width, int height)
{
  Map* map = new Map;
  map->m_width = width;
  map->m_height = height;
  map->m_tileset = cache::loadTexture(other->m_tilesetName);
  map->m_tilesetName = other->m_tilesetName;

  if (!map->m_tileset)
  {
    TRACE("Unable to load tileset: %s", other->m_tilesetName.c_str());
    return 0;
  }

  auto addLayer = [map](const std::string& layerName)
    {
      map->m_tiles[layerName] = new Tile[map->m_width * map->m_height]();

      for (size_t j = 0; j < (size_t)(map->m_width * map->m_height); j++)
      {
        int tileId = -1;

        Tile tile;
        tile.solid = false;
        tile.tileX = 0;
        tile.tileY = 0;

        tile.tileId = tileId;

        map->m_tiles[layerName][j] = tile;
      }
    };

  addLayer("wall");
  addLayer("floor");
  addLayer("ceiling");

  return map;
}

Tile* Map::getTileAt(int x, int y, const std::string& layer)
{
  if (x < 0 || y < 0 || x >= m_width || y >= m_height || m_tiles.count(layer) == 0)
    return 0;

  int index = y * m_width + x;
  if (index >= 0 && index < getNumberOfTiles())
  {
    return &m_tiles[layer][index];
  }
  return 0;
}

void Map::setTileAt(int x, int y, const std::string& layer, int tileId)
{
  Tile* tile = getTileAt(x, y, layer);

  if (tile)
  {
    tile->tileId = tileId;
    tile->tileX = tileId % (m_tileset->getSize().x / config::TILE_W);
    tile->tileY = tileId / (m_tileset->getSize().x / config::TILE_H);
  }
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

bool Map::trapAt(int x, int y) const
{
  for (auto it = m_traps.begin(); it != m_traps.end(); ++it)
  {
    if (it->x == x && it->y == y && !Persistent::instance().isSet(getTrapKey(&(*it))))
    {
      return true;
    }
  }

  return false;
}

const Trap* Map::getTrapAt(int x, int y) const
{
  for (auto it = m_traps.begin(); it != m_traps.end(); ++it)
  {
    if (it->x == x && it->y == y)
    {
      return &(*it);
    }
  }

  return 0;
}

void Map::disableTrap(const Trap* trap)
{
  // Mark this trap as "sprung" in the persistent storage so that it can be
  // saved.
  Persistent::instance().set(getTrapKey(trap), 1);
}

bool Map::blocking(int x, int y)
{
  Tile* tile = getTileAt(x, y, "wall");

  if (tile && tile->tileId > 0)
    return true;

  tile = getTileAt(x, y, "floor");
  if (tile && tile->solid)
    return true;

  return false;
}

std::string Map::xmlDump() const
{
  std::ostringstream xml;

  xml << "<map name=\"" << getName() << "\">\n";

  xml << " <entities>\n";
  for (auto it = m_entities.begin(); it != m_entities.end(); ++it)
  {
    xml << (*it)->xmlDump();
  }
  xml << " </entities>\n";

  for (auto it = s_explored.begin(); it != s_explored.end(); ++it)
  {
    xml << " <explored name=\"" << it->first << "\">\n";
    size_t cnt = 0;
    for (bool explored : it->second)
    {
      xml << explored;

      if (cnt < it->second.size() - 1)
        xml << ",";
      else
        xml << "\n";

      cnt++;
    }
    xml << " </explored>\n";
  }

  xml << "</map>\n";

  return xml.str();
}

const Encounter* Map::checkEncounter()
{
  std::string encounter;

  int range = random_range(0, m_encounterRate);
  if (range == 0)
  {
    std::vector<std::string> encountersCopy = m_encounters;

    if (encountersCopy.size() > 0)
    {
      std::random_shuffle(encountersCopy.begin(), encountersCopy.end());
      encounter = encountersCopy.front();
    }
  }

  return get_encounter(encounter);
}

bool Map::inside(int x, int y) const
{
  return x >= 0 && y >= 0 && x < getWidth() && y < getHeight();
}

std::vector<sf::Image> Map::getTilesetImages() const
{
  int w = m_tileset->getSize().x / config::TILE_W;
  int h = m_tileset->getSize().y / config::TILE_H;

  sf::Image tempImage = m_tileset->copyToImage();

  std::vector<sf::Image> images;

  for (int y = 0; y < h; y++)
  {
    for (int x = 0; x < w; x++)
    {
      sf::Image image;
      image.create(config::TILE_W, config::TILE_H, sf::Color::Transparent);
      image.copy(tempImage, 0, 0, sf::IntRect(x * config::TILE_W, y * config::TILE_H, config::TILE_W, config::TILE_H), true);
      images.push_back(image);
    }
  }

  return images;
}

std::string Map::getTrapKey(const Trap* trap) const
{
  return "Trap[" + getName() + "," + toString(trap->x) + "," + toString(trap->y) + "]";
}

void Map::explore(int x, int y)
{
//  if (isExplored(x, y))
//    return;

  for (int py = y - 1; py <= y + 1; py++)
  {
    for (int px = x - 1; px <= x + 1; px++)
    {
      int index = py * m_width + px;

      if (index >= 0 && index < (m_width * m_height))
      {
        s_explored[getName()][index] = true;
      }
    }
  }
}

bool Map::isExplored(int x, int y) const
{
  int index = y * m_width + x;

  if (index < 0 || index >= (m_width * m_height))
    return false;

  return s_explored[getName()][index];
}

void Map::updateExplored(const std::string& mapName, const std::vector<bool>& explored)
{
  s_explored[mapName] = explored;
}

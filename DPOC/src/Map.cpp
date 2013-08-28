#include <sstream>
#include <fstream>
#include <set>

#include "Direction.h"
#include "Utility.h"
#include "TiledLoader.h"
#include "Cache.h"
#include "logger.h"
#include "Map.h"

static std::string make_tag(int index, const std::string& name)
{
  std::ostringstream ss;
  ss << index << "@@" << name;
  return ss.str();
}

static void compute_sprite_data(sf::Texture* texture,
    int tileX, int tileY,
    int tileWidth, int tileHeight,
    int& spriteSheetX, int& spriteSheetY, Direction& startDirection)
{
  int numberOfTilesX = texture->getSize().x / tileWidth;
  int numberOfTilesY = texture->getSize().y / tileHeight;

  int numberOfBlocksX = (texture->getSize().x / (tileWidth * config::NUM_SPRITES_X));
  int numberOfBlocksY = (texture->getSize().y / (tileHeight * config::NUM_SPRITES_Y));

  spriteSheetX = 0;
  spriteSheetY = 0;

  startDirection = DIR_DOWN;

  // haha.
  for (int blockY = 0; blockY < numberOfBlocksY; blockY++)
  {
    for (int blockX = 0; blockX < numberOfBlocksX; blockX++)
    {
      int sx = blockX * config::NUM_SPRITES_X;
      int sy = blockY * config::NUM_SPRITES_Y;

      if (tileX >= sx && tileY >= sy &&
          tileX < (sx + config::NUM_SPRITES_X) && tileY < (sy + config::NUM_SPRITES_Y))
      {
        spriteSheetX = blockX;
        spriteSheetY = blockY;
        startDirection = (Direction)(tileY % 4);

        return;
      }
    }
  }
}

Map::Map()
 : m_width(0),
   m_height(0),
   m_encounterRate(0)
{
//  for (int i = 0; i < config::MAX_LAYERS; i++)
//  {
//    m_tiles[i] = 0;
//  }
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
  if (loader.loadFromFile(filename))
  {
    map = new Map;
    map->m_name = filename;
    map->m_music = loader.getProperty("music");

    std::set<int> zones;
    map->m_encounterRate = 30;
    if (!loader.getProperty("encounterRate").empty())
    {
      map->m_encounterRate = fromString<int>(loader.getProperty("encounterRate"));
    }

    TRACE("Map: Loading tilesets");

    std::vector<std::string> tilesets = loader.getTilesets();
    for (auto it = tilesets.begin(); it != tilesets.end(); ++it)
    {
      const TiledLoader::Tileset* tileset = loader.getTileset(*it);
      if (tileset->startTileIndex == 1)
      {
        TRACE("Map: loading tileset %s", ("Resources/Maps/" + tileset->tilesetSource).c_str());

        map->m_tileset = cache::loadTexture("Resources/Maps/" + tileset->tilesetSource);

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
        tile.zone = 0;
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
          std::string spriteSheet = "Resources/Maps/" + tileset->tilesetSource;
          sf::Texture* texture = cache::loadTexture(spriteSheet);

          std::string name = object->name;
          float walkSpeed = fromString<float>(loader.getObjectProperty(objectIndex, "walkSpeed"));
          std::string walkThroughProperty = loader.getObjectProperty(objectIndex, "walkThrough");

          int objX = object->x / config::TILE_W;
          int objY = object->y / config::TILE_H;

          int tileX = tileId % (texture->getSize().x / config::TILE_W);
          int tileY = tileId / (texture->getSize().x / config::TILE_H);

          int spriteTileW = tileset->tileW;
          int spriteTileH = tileset->tileH;

          if (name.empty())
          {
            name = "anonymous_object@[" + toString(objX) + "," + toString(objY) + "]";
          }

          Entity* entity = new Entity;
          entity->setPosition(objX, objY);
          entity->setTag(name + "@@" + map->m_name);
          entity->setWalkSpeed(walkSpeed);

          if (walkThroughProperty == "true")
          {
            entity->setWalkThrough(true);
          }

          entity->loadScripts(
              loader.getObjectProperty(objectIndex, "talkScript"),
              loader.getObjectProperty(objectIndex, "stepScript"));

          if (texture == map->m_tileset)
          {
            TileSprite* tileSprite = new TileSprite(map->m_tileset, tileX, tileY);
            entity->setSprite(tileSprite);
          }
          else
          {
            int spriteSheetX, spriteSheetY;
            Direction startDirection;

            compute_sprite_data(texture,
                tileX, tileY,
                spriteTileW, spriteTileH,
                spriteSheetX, spriteSheetY, startDirection);

            Sprite* entitySprite = new Sprite;
            entitySprite->create(spriteSheet, spriteSheetX, spriteSheetY, spriteTileW, spriteTileH);
            entitySprite->setDirection(startDirection);
            entity->setSprite(entitySprite);
            entity->setDirection(startDirection);

            std::string fixDir = loader.getObjectProperty(objectIndex, "fixedDirection");
            if (fixDir == "true")
            {
              entity->setFixedDirection(true);
            }
          }

          map->m_entities.push_back(entity);

          cache::releaseTexture(texture);
        }
      }
      else
      {
        std::string name = object->name;
        if (to_lower(name) == "warp")
        {
          Warp warp;
          warp.srcX = object->x / config::TILE_W;
          warp.srcY = object->y / config::TILE_H;
          warp.dstX = fromString<int>(loader.getObjectProperty(objectIndex, "destX"));
          warp.dstY = fromString<int>(loader.getObjectProperty(objectIndex, "destY"));
          warp.destMap = loader.getObjectProperty(objectIndex, "destMap");
          map->m_warps.push_back(warp);

          TRACE("New warp: srcX=%d, srcY=%d, dstX=%d, dstY=%d, dstMap=%s",
              warp.srcX, warp.srcY, warp.dstX, warp.dstY, warp.destMap.c_str());
        }
        else if (to_lower(name) == "zone")
        {
          int zoneId = fromString<int>(loader.getObjectProperty(objectIndex, "zoneId"));

          zones.insert(zoneId);

          int x = object->x / config::TILE_W;
          int y = object->y / config::TILE_H;
          int w = object->width / config::TILE_W;
          int h = object->height / config::TILE_H;

          for (int py = y; py < y + h; py++)
          {
            for (int px = x; px < x + w; px++)
            {
              int index = py * map->m_width + px;
              map->m_tiles["floor"][index].zone = zoneId;
            }
          }

          TRACE("New zone: x=%d, y=%d, width=%d, height=%d",
              x, y, w, h);
        }
      }
    }

    TRACE("Reading encounters");
    for (auto it = zones.begin(); it != zones.end(); ++it)
    {
      std::string enc = loader.getProperty("zone:" + toString(*it));
      std::vector<std::string> groups = split_string(enc, '|');
      for (auto groupIt = groups.begin(); groupIt != groups.end(); ++groupIt)
      {
        std::vector<std::string> monsters = split_string(*groupIt, ',');
        map->m_encounters.insert(std::make_pair(*it, monsters));
      }
    }
    for (auto it = map->m_encounters.begin(); it != map->m_encounters.end(); ++it)
    {
      std::string buff;
      for (auto jit = it->second.begin(); jit != it->second.end(); ++jit)
      {
        buff += (*jit) + " ";
      }
      TRACE(" - %d: %s", it->first, buff.c_str());
    }

    TRACE("Map loading completed!");
  }
  else
  {
    TRACE("Unable to open tiled file %s", filename.c_str());
  }

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

bool Map::blocking(int x, int y)
{
  Tile* tile = getTileAt(x, y, "floor");

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

  xml << "</map>\n";

  return xml.str();
}

std::vector<std::string> Map::checkEncounter(int x, int y)
{
  std::vector<std::string> monsters;

  int range = random_range(0, m_encounterRate);
  if (range == 0)
  {
    int zone = getTileAt(x, y, "floor")->zone;

    std::vector< std::vector<std::string> > potentials;

    auto itPair = m_encounters.equal_range(zone);
    for (auto it = itPair.first; it != itPair.second; ++it)
    {
      potentials.push_back(it->second);
    }

    if (potentials.size() > 0)
    {
      std::random_shuffle(potentials.begin(), potentials.end());
      monsters = potentials.front();
    }
  }

  return monsters;
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
      image.create(config::TILE_W, config::TILE_H);
      image.copy(tempImage, 0, 0, sf::IntRect(x * config::TILE_W, y * config::TILE_H, config::TILE_W, config::TILE_H));
      images.push_back(image);
    }
  }

  return images;
}

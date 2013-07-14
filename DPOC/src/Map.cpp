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

static void compute_sprite_data(sf::Texture* texture, int tileX, int tileY, int& spriteSheetX, int& spriteSheetY, Direction& startDirection)
{
  int numberOfTilesX = texture->getSize().x / config::TILE_W;
  int numberOfTilesY = texture->getSize().y / config::TILE_H;

  int numberOfBlocksX = (texture->getSize().x / (config::TILE_W * config::NUM_SPRITES_X));
  int numberOfBlocksY = (texture->getSize().y / (config::TILE_H * config::NUM_SPRITES_Y));

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
      for (size_t layer = 0; layer < m_tiles.size(); layer++)
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
              << m_tiles[i][j].zone << " "
              << m_tiles[i][j].solid << " ";
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

    for (size_t i = 0; i < config::MAX_LAYERS; i++)
    {
      map->m_tiles.push_back(0);
    }

    int layers;

    ifile >> map->m_width >> map->m_height;
    ifile >> layers;

    for (int i = 0; i < layers; i++)
    {
      map->m_tiles[i] = new Tile[map->m_width * map->m_height]();
      for (int j = 0; j < map->m_width * map->m_height; j++)
      {
        Tile tile;
        ifile >> tile.tileX >> tile.tileY >> tile.zone >> tile.solid;

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
      entity->setPosition(entX, entY);
      entity->setTag(make_tag(i, map->m_name));

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

      const TiledLoader::Layer* layer = loader.getLayer(layers[i]);

      map->m_width = layer->width;
      map->m_height = layer->height;

      map->m_tiles.push_back(0);
      map->m_tiles.back() = new Tile[map->m_width * map->m_height]();

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

        map->m_tiles.back()[j] = tile;
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
            map->m_tiles.front()[i].solid = true;
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

            compute_sprite_data(texture, tileX, tileY, spriteSheetX, spriteSheetY, startDirection);

            Sprite* entitySprite = new Sprite;
            entitySprite->create(spriteSheet, spriteSheetX, spriteSheetY);
            entitySprite->setDirection(startDirection);
            entity->setSprite(entitySprite);
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
              map->m_tiles[0][index].zone = zoneId;
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

bool Map::blocking(int x, int y)
{
  for (int i = 0; i < config::MAX_LAYERS; i++)
  {
    Tile* tile = getTileAt(x, y, i);

    if (tile && tile->solid)
      return true;
  }

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
    int zone = getTileAt(x, y, 0)->zone;

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

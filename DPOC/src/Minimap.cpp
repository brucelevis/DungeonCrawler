#include "Cache.h"
#include "Map.h"
#include "Player.h"
#include "Game.h"
#include "draw_text.h"
#include "Drawing.h"
#include "Minimap.h"

namespace
{
  const int TILE_SIZE = 8;

  Entity* _entity_at(const std::vector<Entity*>& entities, int x, int y)
  {
    for (auto it = entities.begin(); it != entities.end(); ++it)
    {
      if ((int)(*it)->x == x && (int)(*it)->y == y)
      {
        return *it;
      }
    }

    return nullptr;
  }

  const int TileId_WallMarker = 1;
  const int TileId_Floor      = 2;
  const int TileId_Obstacle   = 6;
  const int TileId_Door       = 14;
  const int TileId_Chest      = 25;
}

Minimap::Minimap(int x, int y, int w, int h)
 : m_x(x),
   m_y(y),
   m_w(w),
   m_h(h),
   m_centerX(0),
   m_centerY(0),
   m_playerX(0),
   m_playerY(0),
   m_currentMap(0),
   m_mapTiles(cache::loadTexture("UI/MapTiles.png")),
   m_arrowTexture(cache::loadTexture("UI/MapMarkerArrow.png"))
{
}

Minimap::~Minimap()
{
  cache::releaseTexture(m_mapTiles);
  cache::releaseTexture(m_arrowTexture);
}

void Minimap::updatePosition(Map* currentMap, int x, int y, int playerX, int playerY)
{
  m_currentMap = currentMap;
  m_centerX = x;
  m_centerY = y;
  m_playerX = playerX;
  m_playerY = playerY;
}

void Minimap::draw(sf::RenderTarget& target) const
{
  if (!m_currentMap)
    return;

  int numberX = m_w / TILE_SIZE;
  int numberY = m_h / TILE_SIZE;

  auto entities = m_currentMap->getEntities();

  // If even number, need to adjust the check below.
  int addX = ((numberX % 2) == 0) ? -1 : 0;
  int addY = ((numberY % 2) == 0) ? -1 : 0;

  for (int y = m_centerY - numberY / 2, py = 0; y <= m_centerY + numberY / 2 + addY; y++, py++)
  {
    for (int x = m_centerX - numberX / 2, px = 0; x <= m_centerX + numberX / 2 + addX; x++, px++)
    {
      int tx = m_x + px * TILE_SIZE;
      int ty = m_y + py * TILE_SIZE;

      if (x < 0 || y < 0 || x >= m_currentMap->getWidth() || y >= m_currentMap->getHeight())
      {
        draw_rectangle(target, tx, ty, TILE_SIZE, TILE_SIZE, sf::Color::Black);
        continue;
      }

      if (!m_currentMap->isExplored(x, y))
      {
        draw_rectangle(target, tx, ty, TILE_SIZE, TILE_SIZE, sf::Color::Black);
        continue;
      }

      Tile* tile = m_currentMap->getTileAt(x, y, "wall");

      if (tile && tile->tileId > -1)
      {
        drawTile(target, TileId_WallMarker, tx, ty, sf::Color::Blue);
      }
      else if (m_currentMap->blocking(x, y))
      {
        drawTile(target, TileId_Obstacle, tx, ty, sf::Color::Red);
      }
      else
      {
        // Probably a floor.
        drawTile(target, TileId_Floor, tx, ty, sf::Color(0, 127, 255));
      }

      if (Entity* entity = _entity_at(entities, x, y))
      {
        if (entity->getName() == "chest")
        {
          drawTile(target, TileId_Chest, tx, ty, sf::Color::Yellow);
        }
        else if (entity->getType() == "door")
        {
          drawTile(target, TileId_Door, tx, ty, sf::Color{0x3a, 0x2a, 0x00});
        }
        else if (entity->getType() == "obstacle")
        {
          drawTile(target, TileId_Obstacle, tx, ty, sf::Color::Red);
        }
        else
        {
          draw_text_bmp(target, tx, ty, "?");
        }
      }

      if (x == m_playerX && y == m_playerY)
      {
        drawArrow(target, get_player()->player()->getDirection(), tx + TILE_SIZE/2, ty + TILE_SIZE/2);
      }
    }
  }

  for (int y = 1; y < numberY; y++)
  {
    sf::RectangleShape rect;
    rect.setSize(sf::Vector2f(m_w, 1));
    rect.setPosition(m_x, m_y + y * TILE_SIZE);
    rect.setFillColor(sf::Color::White);
    target.draw(rect);
  }

  for (int x = 1; x < numberX; x++)
  {
    sf::RectangleShape rect;
    rect.setSize(sf::Vector2f(1, m_h));
    rect.setPosition(m_x + x * TILE_SIZE, m_y);
    rect.setFillColor(sf::Color::White);
    target.draw(rect);
  }

  sf::RectangleShape minimap;
  minimap.setSize(sf::Vector2f(m_w, m_h));
  minimap.setFillColor(sf::Color::Transparent);
  minimap.setOutlineColor(sf::Color::White);
  minimap.setOutlineThickness(1);
  minimap.setPosition(m_x, m_y);
  target.draw(minimap);
}

void Minimap::drawArrow(sf::RenderTarget& target, Direction direction, int tx, int ty) const
{
  sf::Sprite sprite;
  sprite.setTexture(*m_arrowTexture);
  sprite.setOrigin(m_arrowTexture->getSize().x / 2, m_arrowTexture->getSize().y / 2);
  sprite.rotate(-getAngleFromDirection(direction));
  sprite.setPosition(tx, ty);
  target.draw(sprite);
}

void Minimap::drawTile(sf::RenderTarget& target, int tileId, int tx, int ty, const sf::Color& color) const
{
  int numTilesX = m_mapTiles->getSize().x / TILE_SIZE;

  int tileX = tileId % numTilesX;
  int tileY = tileId / numTilesX;

  draw_texture(target, m_mapTiles, tileX * TILE_SIZE, tileY * TILE_SIZE, TILE_SIZE, TILE_SIZE, tx, ty, color);
}

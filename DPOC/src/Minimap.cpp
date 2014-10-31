#include "Cache.h"
#include "Map.h"
#include "Player.h"
#include "Game.h"
#include "draw_text.h"
#include "Minimap.h"

namespace
{
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

  void drawRectangle(sf::RenderTarget& target, int x, int y, const sf::Color& color)
  {
    sf::RectangleShape dotRect;
    dotRect.setSize(sf::Vector2f(8, 8));
    dotRect.setFillColor(color);
    dotRect.setPosition(x, y);
    target.draw(dotRect);
  }

  void drawIcon(sf::RenderTarget& target, sf::Texture* texture, int x, int y)
  {
    sf::Sprite sprite;
    sprite.setTexture(*texture);
    sprite.setPosition(x, y);
    target.draw(sprite);
  }
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
   m_campsiteIcon( cache::loadTexture("Icons/Campfire.png") ),
   m_doorIcon( cache::loadTexture("Icons/Door.png") )
{
}

Minimap::~Minimap()
{
  cache::releaseTexture(m_campsiteIcon);
  cache::releaseTexture(m_doorIcon);
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

  int numberX = m_w / 8;
  int numberY = m_h / 8;

  auto entities = m_currentMap->getEntities();

  for (int y = m_centerY - numberY / 2, py = 0; y <= m_centerY + numberY / 2; y++, py++)
  {
    for (int x = m_centerX - numberX / 2, px = 0; x <= m_centerX + numberX / 2; x++, px++)
    {
      Tile* tile = m_currentMap->getTileAt(x, y, "wall");

      int tx = m_x + px * 8;
      int ty = m_y + py * 8;

      if (tile && tile->tileId > -1)
      {
        drawRectangle(target, tx, ty, sf::Color::Blue);
      }
      else if (m_currentMap->blocking(x, y))
      {
        drawRectangle(target, tx, ty, sf::Color::Red);
      }
      if (x == m_playerX && y == m_playerY)
      {
        drawRectangle(target, tx, ty, sf::Color::Green);

        sf::RectangleShape dotRect;

        auto player = Game::instance().getPlayer()->player();
        switch (player->getDirection())
        {
        case DIR_LEFT:
          dotRect.setSize(sf::Vector2f(4, 1));
          dotRect.setFillColor(sf::Color::Black);
          dotRect.setPosition(tx, ty + 4);
          break;
        case DIR_RIGHT:
          dotRect.setSize(sf::Vector2f(4, 1));
          dotRect.setFillColor(sf::Color::Black);
          dotRect.setPosition(tx + 4, ty + 4);
          break;
        case DIR_UP:
          dotRect.setSize(sf::Vector2f(1, 4));
          dotRect.setFillColor(sf::Color::Black);
          dotRect.setPosition(tx + 4, ty);
          break;
        case DIR_DOWN:
          dotRect.setSize(sf::Vector2f(1, 4));
          dotRect.setFillColor(sf::Color::Black);
          dotRect.setPosition(tx + 4, ty + 4);
          break;
        default:
          break;
        }

        target.draw(dotRect);
      }

      if (Entity* entity = _entity_at(entities, x, y))
      {
        if (entity->getName() == "campsite")
        {
          drawIcon(target, m_campsiteIcon, tx, ty);
        }
        else if (entity->getType() == "door")
        {
          drawIcon(target, m_doorIcon, tx, ty);
        }
        else if (entity->getType() == "obstacle")
        {
          drawRectangle(target, tx, ty, sf::Color::Red);
        }
        else
        {
          draw_text_bmp(target, tx, ty, "?");
        }
      }

    }
  }
  for (int y = 1; y < numberY; y++)
  {
    sf::RectangleShape rect;
    rect.setSize(sf::Vector2f(m_w, 1));
    rect.setPosition(m_x, m_y + y * 8);
    rect.setFillColor(sf::Color::White);
    target.draw(rect);
  }
  for (int x = 1; x < numberX; x++)
  {
    sf::RectangleShape rect;
    rect.setSize(sf::Vector2f(1, m_h));
    rect.setPosition(m_x + x * 8, m_y);
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

#include "Map.h"
#include "Minimap.h"

Minimap::Minimap(int x, int y, int w, int h)
 : m_x(x),
   m_y(y),
   m_w(w),
   m_h(h),
   m_centerX(0),
   m_centerY(0),
   m_currentMap(0)
{
}

void Minimap::updatePosition(Map* currentMap, int x, int y)
{
  m_currentMap = currentMap;
  m_centerX = x;
  m_centerY = y;
}

void Minimap::draw(sf::RenderTarget& target) const
{
  if (!m_currentMap)
    return;

  int numberX = m_w / 8;
  int numberY = m_h / 8;

  for (int y = m_centerY - numberY / 2, py = 0; y <= m_centerY + numberY / 2; y++, py++)
  {
    for (int x = m_centerX - numberX / 2, px = 0; x <= m_centerX + numberX / 2; x++, px++)
    {
      Tile* tile = m_currentMap->getTileAt(x, y, "wall");
      if (tile && tile->tileId > -1)
      {
        sf::RectangleShape dotRect;
        dotRect.setSize(sf::Vector2f(8, 8));
        dotRect.setFillColor(sf::Color::Blue);
        dotRect.setPosition(m_x + px * 8, m_y + py * 8);
        target.draw(dotRect);
      }
      if (x == m_centerX && y == m_centerY)
      {
        sf::RectangleShape dotRect;
        dotRect.setSize(sf::Vector2f(8, 8));
        dotRect.setFillColor(sf::Color::Green);
        dotRect.setPosition(m_x + px * 8, m_y + py * 8);
        target.draw(dotRect);
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

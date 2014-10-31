#include "Player.h"
#include "Config.h"
#include "Map.h"
#include "DungeonMap.h"

static const int WIDTH = 168;
static const int HEIGHT = 168;

DungeonMap::DungeonMap(Map* map)
 : m_map(map),
   m_minimap( config::GAME_RES_X / 2 - WIDTH / 2,
              config::GAME_RES_Y / 2 - HEIGHT / 2,
              WIDTH,
              HEIGHT ),
   m_centerX(get_player()->player()->x),
   m_centerY(get_player()->player()->y)
{
  m_minimap.updatePosition(m_map, m_centerX, m_centerY);
}

void DungeonMap::update()
{
  m_minimap.updatePosition(m_map, m_centerX, m_centerY);
}

void DungeonMap::draw(sf::RenderTarget& target)
{
  m_minimap.draw(target);
}

void DungeonMap::handleEvent(sf::Event& event)
{
  if (event.type == sf::Event::KeyPressed)
  {
    if (event.key.code == sf::Keyboard::Right)
    {
      m_centerX++;

      if (m_centerX >= m_map->getWidth())
      {
        m_centerX = m_map->getWidth() - 1;
      }
    }
    else if (event.key.code == sf::Keyboard::Left)
    {
      m_centerX--;

      if (m_centerX < 0)
      {
        m_centerX = 0;
      }
    }
    else if (event.key.code == sf::Keyboard::Down)
    {
      m_centerY++;

      if (m_centerY >= m_map->getHeight())
      {
        m_centerY = m_map->getHeight() - 1;
      }
    }
    else if (event.key.code == sf::Keyboard::Up)
    {
      m_centerY--;

      if (m_centerY < 0)
      {
        m_centerY = 0;
      }
    }
    else if (event.key.code == sf::Keyboard::Escape)
    {
      close();
    }
  }
}

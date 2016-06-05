#include "Cache.h"
#include "Player.h"
#include "Config.h"
#include "Map.h"
#include "Frame.h"
#include "Drawing.h"

#include "DungeonMap.h"

static const int WIDTH = 128;
static const int HEIGHT = 96;

#define POS_X (config::GAME_RES_X / 2 - WIDTH / 2)
#define POS_Y (config::GAME_RES_Y / 2 - HEIGHT / 2)

DungeonMap::DungeonMap(Map* map)
 : m_map(map),
   m_minimap( POS_X,
              POS_Y,
              WIDTH,
              HEIGHT ),
   m_centerX(get_player()->player()->x),
   m_centerY(get_player()->player()->y),
   m_arrowTexture(cache::loadTexture("UI/Arrow_Big.png")),
   m_mapTexture(cache::loadTexture("UI/Map.png"))
{
  m_minimap.updatePosition(m_map, m_centerX, m_centerY, m_centerX, m_centerY);
}

DungeonMap::~DungeonMap()
{
  cache::releaseTexture(m_arrowTexture);
  cache::releaseTexture(m_mapTexture);
}

void DungeonMap::update()
{
  m_minimap.updatePosition(m_map, m_centerX, m_centerY, get_player()->player()->x, get_player()->player()->y);
}

void DungeonMap::draw(sf::RenderTarget& target)
{
  const int mapPosX = config::GAME_RES_X / 2 - m_mapTexture->getSize().x / 2;
  const int mapPosY = config::GAME_RES_Y / 2 - m_mapTexture->getSize().y / 2;

  draw_frame(target, 0, 0, config::GAME_RES_X, config::GAME_RES_Y);
  draw_texture(target, m_mapTexture, mapPosX, mapPosY);
  m_minimap.draw(target);

  int left  = POS_X - 19;
  int right = POS_X + WIDTH + 19;
  int top   = POS_Y - 19;
  int bottom = POS_Y + HEIGHT + 19;

  int middle_x = config::GAME_RES_X / 2;
  int middle_y = config::GAME_RES_Y / 2;

  sf::Sprite sprite;
  sprite.setTexture(*m_arrowTexture);
  sprite.setOrigin(m_arrowTexture->getSize().x / 2, m_arrowTexture->getSize().y / 2);

  // RIGHT
  sprite.setPosition(right, middle_y);
  if (m_centerX < (m_map->getWidth() - 1))
  {
    target.draw(sprite);
  }

  // DOWN
  sprite.rotate(90);
  sprite.setPosition(middle_x, bottom);
  if (m_centerY < (m_map->getHeight() - 1))
  {
    target.draw(sprite);
  }

  // LEFT
  sprite.rotate(90);
  sprite.setPosition(left, middle_y);
  if (m_centerX > 0)
  {
    target.draw(sprite);
  }

  // UP
  sprite.rotate(90);
  sprite.setPosition(middle_x, top);
  if (m_centerY > 0)
  {
    target.draw(sprite);
  }
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

#include <algorithm>
#include <sstream>

#include "SceneManager.h"

#include "Sound.h"
#include "Utility.h"
#include "Map.h"
#include "Player.h"
#include "Game.h"
#include "Persistent.h"
#include "Message.h"
#include "logger.h"
#include "Config.h"
#include "Entity.h"

Entity::Entity()
: x(0),
  y(0),
  m_sprite(0),
  m_direction(DIR_DOWN),
  m_speed(0.1),
  m_targetX(0), m_targetY(0),
  m_state(STATE_NORMAL),
  m_waitCounter(0),
  m_walkThrough(false),
  m_visible(true),
  m_fixedDirection(false)
{
}

Entity::Entity(const std::string& name)
 : x(0),
   y(0),
   m_name(name),
   m_sprite(0),
   m_direction(DIR_DOWN),
   m_speed(0.1),
   m_targetX(0), m_targetY(0),
   m_state(STATE_NORMAL),
   m_waitCounter(0),
   m_walkThrough(false),
   m_visible(true),
   m_fixedDirection(false)
{
}

Entity::~Entity()
{
  delete m_sprite;
}

void Entity::loadScripts(const std::string& talkScript, const std::string& stepScript, const std::vector<std::string>& arguments)
{
  TRACE("Entity[%s]::loadScripts(%s, %s)", getTag().c_str(), talkScript.c_str(), stepScript.c_str());

  if (!talkScript.empty())
  {
    m_script.loadFromFile(config::res_path("Scripts/" + talkScript), arguments);
    m_script.setCallingEntity(this);
  }

  if (!stepScript.empty())
  {
    m_script.loadFromFile(config::res_path("Scripts/" + stepScript), arguments);
    m_stepScript.setCallingEntity(this);
    m_stepScript.execute();
  }
}

void Entity::update()
{
  if (m_state == STATE_WALKING)
  {
    walk();

    if (m_sprite)
    {
      m_sprite->update(m_direction);
    }
  }
  else if (m_state == STATE_WAITING)
  {
    m_waitCounter--;
    if (m_waitCounter == 0)
    {
      m_state = STATE_NORMAL;
    }
  }

  if (m_scriptWaitMap[&m_script] > 0)
  {
    m_scriptWaitMap[&m_script]--;
  }

  if (m_scriptWaitMap[&m_stepScript] > 0)
  {
    m_scriptWaitMap[&m_stepScript]--;
  }

  if (m_state != STATE_WALKING)
  {
    if (m_script.active() && m_scriptWaitMap[&m_script] == 0)
    {
      m_script.next();
    }

    if (!m_script.active() && m_scriptWaitMap[&m_stepScript] == 0)
    {
      if (m_stepScript.active())
      {
        m_stepScript.next();

        if (!m_stepScript.active())
        {
          m_stepScript.execute();
        }
      }
      else if (!m_stepScript.active())
      {
        m_stepScript.execute();
      }
    }
  }
}

void Entity::setDirection(Direction dir)
{
  if (m_fixedDirection)
    return;

  if (dir == DIR_RANDOM)
  {
    m_direction = (Direction)(rand()%4);
  }
  else
  {
    m_direction = dir;
  }

  if (m_sprite)
  {
    m_sprite->setDirection(m_direction);
  }
}

void Entity::step(Direction dir)
{
  if (m_state != STATE_WALKING)
  {
    setDirection(dir);

    if (m_direction == DIR_RIGHT)
      m_targetX = x + 1;
    else if (m_direction == DIR_LEFT)
      m_targetX = x - 1;
    else if (m_direction == DIR_DOWN)
      m_targetY = y + 1;
    else if (m_direction == DIR_UP)
      m_targetY = y - 1;

    if (!m_walkThrough &&
        (Game::instance().getCurrentMap()->blocking(m_targetX, m_targetY) ||
         !Game::instance().getCurrentMap()->inside(m_targetX, m_targetY) ||
         checkPlayerCollision() ||
         checkEntityCollision()))
    {
      m_targetX = x;
      m_targetY = y;
    }
    else
    {
      m_state = STATE_WALKING;
    }
  }
}

void Entity::walk()
{
  if (m_direction == DIR_RIGHT)
  {
    x += m_speed;
    if (x >= m_targetX)
    {
      x = m_targetX;
      m_state = STATE_NORMAL;
    }
  }
  else if (m_direction == DIR_LEFT)
  {
    x -= m_speed;
    if (x <= m_targetX)
    {
      x = m_targetX;
      m_state = STATE_NORMAL;
    }
  }
  else if (m_direction == DIR_DOWN)
  {
    y += m_speed;
    if (y >= m_targetY)
    {
      y = m_targetY;
      m_state = STATE_NORMAL;
    }
  }
  else if (m_direction == DIR_UP)
  {
    y -= m_speed;
    if (y <= m_targetY)
    {
      y = m_targetY;
      m_state = STATE_NORMAL;
    }
  }
}

void Entity::wait(int duration)
{
  if (m_state != STATE_WALKING)
  {
    m_state = STATE_WAITING;
    m_waitCounter = duration;
  }
}

void Entity::draw(sf::RenderTarget& target, const coord_t& view)
{
  if (m_sprite && m_visible)
  {
    float x = getRealX() - (m_sprite->getWidth() - config::TILE_W);
    float y = getRealY() - (m_sprite->getHeight() - config::TILE_H);

    m_sprite->render(target, x - view.x, y - view.y);
  }
}

float Entity::getRealX() const
{
  return x * config::TILE_W;
}

float Entity::getRealY() const
{
  return y * config::TILE_H;
}

void Entity::interact(const Entity* interactor)
{
  if (interactor)
  {
    face(interactor);
  }

  if (m_script.isLoaded() && !m_script.active())
  {
    m_script.execute();
  }
}

void Entity::face(const Entity* entity)
{
  switch (entity->m_direction)
  {
  case DIR_RIGHT:
    setDirection(DIR_LEFT);
    break;
  case DIR_DOWN:
    setDirection(DIR_UP);
    break;
  case DIR_LEFT:
    setDirection(DIR_RIGHT);
    break;
  case DIR_UP:
    setDirection(DIR_DOWN);
    break;
  default:
    break;
  }
}

bool Entity::canInteractWith(const Entity* interactor) const
{
  if (!isWalking() && !interactor->isWalking())
  {
    int myX = x;
    int myY = y;

    int px = interactor->x;
    int py = interactor->y;

    switch (interactor->m_direction)
    {
    case DIR_RIGHT:
      return (px == myX - 1) && (py == myY);
    case DIR_LEFT:
      return (px == myX + 1) && (py == myY);
    case DIR_DOWN:
      return (px == myX) && (py == myY - 1);
    case DIR_UP:
      return (px == myX) && (py == myY + 1);
    default:
      break;
    }
  }

  return false;
}

bool Entity::checkPlayerCollision() const
{
  if (m_walkThrough)
    return false;

  Entity* player = Game::instance().getPlayer()->player();

  if (player == this || player->m_walkThrough)
    return false;

  return (((int)player->x == x && (int)player->y == y) ||
          (player->getTargetX() == getTargetX() && player->getTargetY() == getTargetY()));
}

bool Entity::checkEntityCollision() const
{
  if (m_walkThrough)
    return false;

  Map* map = Game::instance().getCurrentMap();

  for (auto it = map->getEntities().begin(); it != map->getEntities().end(); ++it)
  {
    if ((*it) == this || (*it)->m_walkThrough)
      continue;

    int px = (*it)->x;
    int py = (*it)->y;

    if (px == x && py == y)
      return true;

    if (px == getTargetX() && py == getTargetY())
      return true;

    if (getTargetX() == (*it)->getTargetX() && getTargetY() == (*it)->getTargetY())
      return true;
  }

  return false;
}

std::string Entity::xmlDump() const
{
  std::ostringstream xml;

  xml << "<entity name=\"" << m_name << "\" tag=\"" << getTag() << "\">\n";

  if (m_sprite)
  {
    xml << " <sprite name=\"" << m_sprite->getTextureName() << "\" />\n";
  }
  xml << " <direction>" << directionToString(m_direction) << "</direction>\n";
  xml << " <speed>" << m_speed << "</speed>\n";
  xml << " <walkThrough>" << m_walkThrough << "</walkThrough>\n";
  xml << " <x>" << (int)x << "</x>\n";
  xml << " <y>" << (int)y << "</y>\n";

  xml << "</entity>\n";

  return xml.str();
}

void Entity::turnLeft()
{
  if (m_direction == DIR_DOWN)       m_direction = DIR_RIGHT;
  else if (m_direction == DIR_UP)    m_direction = DIR_LEFT;
  else if (m_direction == DIR_RIGHT) m_direction = DIR_UP;
  else if (m_direction == DIR_LEFT)  m_direction = DIR_DOWN;
}

void Entity::turnRight()
{
  if (m_direction == DIR_DOWN)       m_direction = DIR_LEFT;
  else if (m_direction == DIR_UP)    m_direction = DIR_RIGHT;
  else if (m_direction == DIR_RIGHT) m_direction = DIR_DOWN;
  else if (m_direction == DIR_LEFT)  m_direction = DIR_UP;
}

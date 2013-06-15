#include <algorithm>

#include "Message.h"
#include "logger.h"
#include "Config.h"
#include "EntityDef.h"
#include "Entity.h"

Entity::Entity(const std::string& name)
 : x(0),
   y(0),
   m_sprite(0),
   m_direction(DIR_DOWN),
   m_speed(0.1),
   m_targetX(0), m_targetY(0),
   m_walking(false)
{
  auto it = std::find_if(ENTITY_DEF.begin(), ENTITY_DEF.end(),
      [=](const EntityDef& entity)
      {
        return entity.name == name;
      });

  if (it != ENTITY_DEF.end())
  {
    m_name = name;
    if (!it->spriteSheet.empty())
    {
      m_sprite = new Sprite;
      m_sprite->create(it->spriteSheet, it->spriteSheetX, it->spriteSheetY);
    }

    if (!it->scriptFile.empty())
    {
      m_script.loadFromFile(it->scriptFile);
    }
  }
  else
  {
    TRACE("Unable to create entity %s! Not found in def array.", name.c_str());
  }
}

Entity::~Entity()
{
  delete m_sprite;
}

void Entity::update()
{
  if (m_script.active())
  {
    executeScriptLine(m_script.getCurrentData());

    m_script.advance();
  }

  if (m_walking)
  {
    walk();

    if (m_sprite)
    {
      m_sprite->update(m_direction);
    }
  }
}

void Entity::setDirection(Direction dir)
{
  if (dir == DIR_RANDOM)
  {
    m_direction = (Direction)(rand()%4);
  }
  else
  {
    m_direction = dir;
  }

  m_sprite->setDirection(m_direction);
}

void Entity::step(Direction dir)
{
  if (!m_walking)
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

    m_walking = true;
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
      m_walking = false;
    }
  }
  else if (m_direction == DIR_LEFT)
  {
    x -= m_speed;
    if (x <= m_targetX)
    {
      x = m_targetX;
      m_walking = false;
    }
  }
  else if (m_direction == DIR_DOWN)
  {
    y += m_speed;
    if (y >= m_targetY)
    {
      y = m_targetY;
      m_walking = false;
    }
  }
  else if (m_direction == DIR_UP)
  {
    y -= m_speed;
    if (y <= m_targetY)
    {
      y = m_targetY;
      m_walking = false;
    }
  }
}

void Entity::draw(sf::RenderTarget& target, const coord_t& view)
{
  if (m_sprite)
  {
    m_sprite->render(target, getRealX() - view.x, getRealY() - view.y);
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
  if (!m_walking && !interactor->m_walking)
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

void Entity::executeScriptLine(const Script::ScriptData& data)
{
  if (data.opcode == Script::OP_MESSAGE)
  {
    Message::instance().show(data.data.messageData.message);

    Script::ScriptData next;
    if (m_script.peekNext(next))
    {
      if (next.opcode == Script::OP_MESSAGE)
      {
        m_script.advance();
        executeScriptLine(next);
      }
    }
  }
}

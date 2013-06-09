#include <algorithm>

#include "logger.h"
#include "EntityDef.h"
#include "Entity.h"

Entity::Entity(const std::string& name)
 : x(0),
   y(0),
   m_sprite(0)
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

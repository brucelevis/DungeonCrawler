#include <algorithm>

#include "EntityDef.h"
#include "Entity.h"

Entity* create_entity(const std::string& name)
{
  auto it = std::find_if(ENTITY_DEF.begin(), ENTITY_DEF.end(),
      [=](const Entity& entity)
      {
        return entity.name == name;
      });

  if (it != ENTITY_DEF.end())
  {
    return new Entity(*it);
  }

  return 0;
}

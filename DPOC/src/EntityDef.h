#ifndef ENTITY_DEF_H
#define ENTITY_DEF_H

#include <vector>
#include "Entity.h"

static std::vector<EntityDef> ENTITY_DEF =
{
  {
    "bug_entity",
    "", 0, 0
  },

  {
    "hero",
    "Resources/Hero.png", 0, 0,
    "Resources/Scripts/test.script"
  },

  {
    "player",
    "Resources/Hero.png", 0, 0
  }
};

#endif

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
    0.05f,
    "Resources/Scripts/test.script"
//    "Resources/Scripts/test-step.script"
  },

  {
    "player",
    "Resources/Hero.png", 0, 0,
    0.1f
  }
};

#endif

#include <stdexcept>

#include "logger.h"

#include "PlayerClass.h"

static std::vector<PlayerClass> classes =
{
  {
    "Hero",
    {
      { "hp",       80 },
      { "mp",       40 },
      { "strength", 80 },
      { "power",    60 },
      { "defense",  60 },
      { "magic",    50 },
      { "mag.def",  50 },
      { "speed",    40 }
    },

    {
      { 1, { "Hurt", "Heal", "Paralyze", "Poison" } },
      { 2, { "Hurt All" } }
    },

    { "Rusty Knife", "Wood Shield" }
  }
};

PlayerClass& player_class_ref(const std::string& className)
{
  for (auto it = classes.begin(); it != classes.end(); ++it)
  {
    if (it->name == className)
    {
      return *it;
    }
  }

  TRACE("No class %s defined", className.c_str());

  throw std::runtime_error("No class " + className + " defined");
}

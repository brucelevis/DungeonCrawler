#include <vector>

#include "logger.h"
#include "Spell.h"

static std::vector<Spell> spells =
{
  {
    "Hurt", 4
  }
};

const Spell* get_spell(const std::string& spell)
{
  for (auto it = spells.begin(); it != spells.end(); ++it)
  {
    if (it->name == spell)
    {
      return &(*it);
    }
  }

  TRACE("ERROR: Trying to get nonexisting spell %s", spell.c_str());

  return 0;
}

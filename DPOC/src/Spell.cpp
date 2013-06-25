#include <vector>

#include "logger.h"
#include "Message.h"
#include "Character.h"
#include "Spell.h"

static std::vector<Spell> spells =
{
  {
    "Hurt", "Low damage to single enemy",
    4,
    Spell::TARGET_SINGLE_ENEMY,
    true
  },

  {
    "Heal", "Heals a single ally",
    4,
    Spell::TARGET_SINGLE_ALLY,
    false
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

void cast_spell(const Spell* spell, Character* caster, Character* target)
{
  Message::instance().show(caster->getName() + " casts " + spell->name + " on " + target->getName() + "!");

  caster->getAttribute("mp").current -= spell->mpCost;
}

bool can_cast_spell(const Spell* spell, Character* caster)
{
  return spell->mpCost < caster->getAttribute("mp").current;
}

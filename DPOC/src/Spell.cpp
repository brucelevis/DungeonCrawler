#include <vector>

#include "Attack.h"
#include "logger.h"
#include "Message.h"
#include "Character.h"
#include "Spell.h"

static std::vector<Spell> spells =
{
  {
    "Hurt", "Low damage to single enemy",
    4,
    TARGET_SINGLE_ENEMY,
    true,

    "Effect_Flame",

    Spell::SPELL_DAMAGE,
    80
  },

  {
    "Hurt All", "Low damage to all enemies",
    7,
    TARGET_ALL_ENEMY,
    true,

    "Effect_Flame",

    Spell::SPELL_DAMAGE,
    60
  },

  {
    "Heal", "Heals a single ally",
    4,
    TARGET_SINGLE_ALLY,
    false,

    "",

    Spell::SPELL_HEAL,
    80
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

int cast_spell(const Spell* spell, Character* caster, Character* target)
{
  int damage = 0;

  if (spell->spellType == Spell::SPELL_DAMAGE || spell->spellType == Spell::SPELL_HEAL)
  {
    damage = calculate_magical_damage(caster, target, spell);
  }

  target->getAttribute("hp").current -= damage;
  if (target->getAttribute("hp").current >= target->getAttribute("hp").max)
  {
    clamp_attribute(target->getAttribute("hp"));
  }

  caster->getAttribute("mp").current -= spell->mpCost;

  return damage;
}

bool can_cast_spell(const Spell* spell, Character* caster)
{
  return spell->mpCost <= caster->getAttribute("mp").current;
}

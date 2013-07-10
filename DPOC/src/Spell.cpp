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
  },

  {
    "Paralyze", "Paralyzes an enemy",
    4,
    TARGET_SINGLE_ENEMY,
    true,
    "",
    Spell::SPELL_CAUSE_STATUS,
    0,
    "Paralyze"
  },

  {
    "Poison", "Poisons an enemy",
    4,
    TARGET_SINGLE_ENEMY,
    true,
    "",
    Spell::SPELL_CAUSE_STATUS,
    0,
    "Poison"
  },

  {
    "Buff", "Increase power",
    4,
    TARGET_SINGLE_ALLY,
    true,
    "",
    Spell::SPELL_BUFF,
    100,
    "strength"
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
  else if (spell->spellType == Spell::SPELL_CAUSE_STATUS)
  {
    cause_status(target, spell->extra);
  }
  else if (spell->spellType == Spell::SPELL_REMOVE_STATUS)
  {
    cure_status(target, spell->extra);
  }
  else if (spell->spellType == Spell::SPELL_BUFF)
  {
    buff(target, spell->extra, spell->power);
  }

  target->takeDamage("hp", damage);

  caster->getAttribute("mp").current -= spell->mpCost;

  return damage;
}

bool can_cast_spell(const Spell* spell, Character* caster)
{
  return spell->mpCost <= caster->getAttribute("mp").current;
}

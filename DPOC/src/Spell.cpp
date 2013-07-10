#include <vector>

#include "Utility.h"
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
    {
      { "Paralyze", 75 }
    }
  },

  {
    "Poison", "Poisons an enemy",
    4,
    TARGET_SINGLE_ENEMY,
    true,
    "",
    Spell::SPELL_CAUSE_STATUS,
    0,
    {
      { "Poison", 100 }
    }
  },

  {
    "Buff", "Increase power",
    4,
    TARGET_SINGLE_ALLY,
    true,
    "",
    Spell::SPELL_BUFF,
    100,
    {},
    { "strength" }
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

  if ((spell->spellType & Spell::SPELL_DAMAGE) || (spell->spellType & Spell::SPELL_HEAL))
  {
    damage = calculate_magical_damage(caster, target, spell);
  }

  if (spell->spellType & Spell::SPELL_CAUSE_STATUS)
  {
    for (auto it = spell->causeStatus.begin(); it != spell->causeStatus.end(); ++it)
    {
      int range = random_range(0, 100);
      if (range < it->second)
      {
        cause_status(target, it->first);
      }
      else
      {
        battle_message("No effect...");
      }
    }
  }

  if (spell->spellType & Spell::SPELL_REMOVE_STATUS)
  {
    for (auto it = spell->causeStatus.begin(); it != spell->causeStatus.end(); ++it)
    {
      cure_status(target, it->first);
    }
  }

  if (spell->spellType & Spell::SPELL_BUFF)
  {
    for (auto it = spell->attributeBuffs.begin(); it != spell->attributeBuffs.end(); ++it)
    {
      buff(target, *it, spell->power);
    }
  }

  target->takeDamage("hp", damage);

  caster->getAttribute("mp").current -= spell->mpCost;

  return damage;
}

bool can_cast_spell(const Spell* spell, Character* caster)
{
  return spell->mpCost <= caster->getAttribute("mp").current;
}

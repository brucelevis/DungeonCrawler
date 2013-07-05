#ifndef SPELL_H
#define SPELL_H

#include <string>

#include "Target.h"

class Character;

struct Spell
{
  std::string name;
  std::string description;
  int mpCost;

  Target target;

  bool battleOnly;

  std::string effect;

  enum
  {
    SPELL_DAMAGE,
    SPELL_HEAL,
    SPELL_BUFF,
    SPELL_STATUS,
    SPELL_CUSTOM
  } spellType;

  int power;

  // Status name/attribute name for buffs.
  std::string extra;
};

const Spell* get_spell(const std::string& spell);
int cast_spell(const Spell* spell, Character* caster, Character* target);
bool can_cast_spell(const Spell* spell, Character* caster);

#endif

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
};

const Spell* get_spell(const std::string& spell);
void cast_spell(const Spell* spell, Character* caster, Character* target);
bool can_cast_spell(const Spell* spell, Character* caster);

#endif

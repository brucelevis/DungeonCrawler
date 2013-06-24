#ifndef SPELL_H
#define SPELL_H

#include <string>

class Character;

struct Spell
{
  std::string name;
  std::string description;
  int mpCost;

  enum
  {
    TARGET_SINGLE_ENEMY,
    TARGET_SINGLE_ALLY,
    TARGET_ALL_ENEMY,
    TARGET_ALL_ALLY,
    TARGET_SELF,
    TARGET_NONE
  } target;

  bool battleOnly;
};

const Spell* get_spell(const std::string& spell);
void cast_spell(const Spell* spell, Character* caster, Character* target);

#endif

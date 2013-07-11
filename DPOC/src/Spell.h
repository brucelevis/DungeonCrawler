#ifndef SPELL_H
#define SPELL_H

#include <map>
#include <string>

#include "Target.h"

class Character;

enum SpellType
{
  SPELL_NONE          = 0,
  SPELL_DAMAGE        = 1,
  SPELL_HEAL          = 2,
  SPELL_BUFF          = 4,
  SPELL_REMOVE_STATUS = 8,
  SPELL_CAUSE_STATUS  = 16,
  SPELL_CUSTOM        = 32
};

struct Spell
{
  std::string name;
  std::string description;
  int mpCost;

  Target target;

  bool battleOnly;

  std::string effect;

  int spellType;

  int power;

  // Status name/attribute name for buffs.
  std::map<std::string, int> causeStatus;
  std::vector<std::string> attributeBuffs;
};

void load_spells();

const Spell* get_spell(const std::string& spell);
int cast_spell(const Spell* spell, Character* caster, Character* target);
bool can_cast_spell(const Spell* spell, Character* caster);

#endif

#ifndef SPELL_H
#define SPELL_H

#include <string>

struct Spell
{
  std::string name;
  int mpCost;
};

const Spell* get_spell(const std::string& spell);

#endif

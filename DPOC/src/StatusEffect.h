#ifndef STATUS_EFFECT_H
#define STATUS_EFFECT_H

#include <string>

struct StatusEffect
{
  std::string name;
  std::string verb;

  int recoveryChance;

  bool incapacitate;

  enum
  {
    DAMAGE_NONE,
    DAMAGE_FIXED,
    DAMAGE_PERCENT
  } damageType;
  int damagePerTurn;
};

#endif

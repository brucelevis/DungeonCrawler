#ifndef STATUS_EFFECT_H
#define STATUS_EFFECT_H

#include <string>

struct StatusEffect
{
  std::string name;
  std::string verb;
  std::string recoverVerb;

  bool battleOnly;

  int recoveryChance;

  bool incapacitate;

  enum
  {
    DAMAGE_NONE,
    DAMAGE_FIXED,
    DAMAGE_PERCENT,
  } damageType;
  std::string damageStat;
  int damagePerTurn;

  std::string sound;
};

StatusEffect* get_status_effect(const std::string& status);

#endif

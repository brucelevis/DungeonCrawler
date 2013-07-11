#ifndef STATUS_EFFECT_H
#define STATUS_EFFECT_H

#include <string>

#include <SFML/Graphics.hpp>

struct StatusEffect
{
  std::string name;
  std::string verb;
  std::string recoverVerb;

  // Color to identify status effect by.
  sf::Color color;

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

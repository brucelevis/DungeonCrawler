#ifndef STATUS_EFFECT_H
#define STATUS_EFFECT_H

#include <string>

#include <SFML/Graphics.hpp>

enum DamageType
{
  DAMAGE_NONE,
  DAMAGE_FIXED,
  DAMAGE_PERCENT,
};

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

  DamageType damageType;
  std::string damageStat;
  int damagePerTurn;

  std::string sound;
};

void load_status_effects();

StatusEffect* get_status_effect(const std::string& status);

#endif

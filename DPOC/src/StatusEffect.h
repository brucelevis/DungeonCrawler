#ifndef STATUS_EFFECT_H
#define STATUS_EFFECT_H

#include <string>

#include <SFML/Graphics.hpp>

#include "Effect.h"

class Character;

enum DamageType
{
  DAMAGE_NONE,
  DAMAGE_FIXED,
  DAMAGE_PERCENT,
};

// Some hard-coded status effect types flags.
enum StatusType
{
  STATUS_NONE       = 0,
  STATUS_CONFUSE    = 1,
  STATUS_FUMBLE     = 2,
  STATUS_BLIND      = 4,
  STATUS_REFLECT    = 8,
  STATUS_PROVOKE    = 16,
  STATUS_SILENCE    = 32
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

  int statusType;

  Effect effect;

  int applyDamage(Character* character) const;
};

void load_status_effects();

StatusEffect* get_status_effect(const std::string& status);

#endif

#include <algorithm>

#include "Utility.h"
#include "Message.h"
#include "StatusEffect.h"

#include "Attack.h"

int attack(Character* attacker, Character* target, bool guard, Item* weapon, bool& wasCritical)
{
  int damage = calculate_physical_damage(attacker, target, weapon);

  int aSpeed = attacker->computeCurrentAttribute("speed");
  int bSpeed = target->computeCurrentAttribute("speed");

  if (aSpeed < bSpeed)
  {
    int speedDelta = bSpeed - aSpeed;

    int range = random_range(0, 255);
    if (range < speedDelta)
    {
      damage = 0;
    }
  }
  else
  {
    int range = random_range(0, 255);
    if (range == 0)
    {
      damage = 0;
    }
  }

  if (attacker->hasStatusType(STATUS_BLIND) && coinflip())
  {
    damage = 0;
  }

  if (guard)
  {
    damage /= 2;
  }

  if (damage > 0)
  {
    bool critical = attacker->computeCurrentAttribute("luck") >= random_range(0, 1024);
    wasCritical = critical;

    if (critical)
    {
      battle_message("Critical hit!!");
      damage *= 3;
    }
  }

  target->takeDamage("hp", damage);

  return damage;
}

int calculate_physical_damage(Character* attacker, Character* target, Item* weapon)
{
  float atk = attacker->computeCurrentAttribute("strength");
  float def = target->computeCurrentAttribute("defense");

  float resist = 1.0f;

  if (weapon)
  {
    for (auto it = weapon->elements.begin(); it != weapon->elements.end(); ++it)
    {
      resist *= target->getResistance(it->first);
    }
  }

  float damage = 0;

  damage = (atk / 2.0f - def / 4.0f) * rand_float(0.8f, 1.2f);

//  if (atk >= (2 + def / 2.0f))
//  {
//    damage = (atk - def / 2.0f + ((atk - def / 2.0f + 1.0f) * (float)random_range(0, 256)) / 256.0f) / 4.0f;
//  }
//  else
//  {
//    float b = std::max(5.0f, atk - (12.0f * (def - atk + 1.0f)) / atk);
//    damage = ((b / 2.0f + 1.0f) * (float)random_range(0, 256) / 256.0f + 2.0f) / 3.0f;
//  }

  if ((int)damage <= 0)
  {
    damage = random_range(0, 2);
  }

  damage *= resist;

  return damage;
}

int calculate_physical_damage_item(Character* attacker, Character* target, Item* usedItem)
{
  if (usedItem->itemUseType == ITEM_HEAL_FIXED)
  {
    return -usedItem->attributeGain["hp"];
  }
  else if (usedItem->itemUseType == ITEM_DAMAGE || usedItem->itemUseType == ITEM_HEAL)
  {
    float atk = usedItem->attributeGain["strength"];
    float def = target->computeCurrentAttribute("defense");

    float damage = 0;

    if (usedItem->itemUseType == ITEM_HEAL)
    {
      def = 0;
    }

    if (atk >= (2 + def / 2.0f))
    {
      damage = (atk - def / 2.0f + ((atk - def / 2.0f + 1.0f) * (float)random_range(0, 256)) / 256.0f) / 4.0f;
    }
    else
    {
      float b = std::max(5.0f, atk - (12.0f * (def - atk + 1.0f)) / atk);
      damage = ((b / 2.0f + 1.0f) * (float)random_range(0, 256) / 256.0f + 2.0f) / 3.0f;
    }

    if ((int)damage <= 0)
    {
      damage = random_range(0, 2);
    }

    if (usedItem->itemUseType == ITEM_HEAL)
    {
      damage = -damage;
    }

    return damage;
  }
  else if (usedItem->itemUseType == ITEM_RESTORE_MP_FIXED)
  {
    return -usedItem->attributeGain["mp"];
  }

  return 0;
}

int calculate_magical_damage(Character* attacker, Character* target, const Spell* spell)
{
  float str = attacker->computeCurrentAttribute("magic");
  float pow = spell->power;
  float def = target->computeCurrentAttribute("mag.def");

  float resistance = target->getResistance(spell->element);

  if (spell->spellType & SPELL_HEAL)
  {
    def = 0;
  }

  float atk = (1.0f + str / 255.0f) * pow;
  float damage = 0;

  damage = (atk / 2.0f - def / 4.0f) * rand_float(0.8f, 1.2f);

//  if (atk >= (2 + def))
//  {
//    damage = (atk - def / 2.0f + ((atk - def / 2.0f + 1.0f) * (float)random_range(0, 256)) / 256.0f) / 4.0f;
//  }
//  else
//  {
//    float b = std::max(5.0f, atk - (12.0f * (def - atk + 1.0f)) / atk);
//    damage = ((b / 2.0f + 1.0f) * (float)random_range(0, 256) / 256.0f + 2.0f) / 3.0f;
//  }

  if ((int)damage <= 0)
  {
    damage = 1;
  }

  damage *= resistance;

  if (spell->spellType & SPELL_HEAL)
  {
    damage = -damage;
  }

  return damage;
}

bool cause_status(Character* target, const std::string& status, bool forceStatus, int duration)
{
  bool should = !target->isImmune(status);

  if (forceStatus)
    should = true;

  if (target->getStatus() != "Dead" && !target->hasStatus(status) && should)
  {
    target->afflictStatus(status, duration);

    battle_message("%s %s",
        target->getName().c_str(), get_status_effect(status)->verb.c_str());
  }
  else
  {
    return false;
  }

  return true;
}

void cure_status(Character* target, const std::string& status)
{
  if (target->hasStatus(status))
  {
    target->cureStatus(status);

    battle_message("%s %s",
        target->getName().c_str(), get_status_effect(status)->recoverVerb.c_str());
  }
  else
  {
    battle_message("No effect...");
  }
}

void buff(Character* target, const std::string& attr, int buffPower)
{
  int baseAttr = target->getAttribute(attr).max;
  float power = 1.0f + (float)buffPower / 100.0f;
  int newAttr = (float)baseAttr * power;

  target->getAttribute(attr).current = newAttr;

  int delta = newAttr - baseAttr;

  if (delta > 0)
  {
    battle_message("%s's %s increased by %d",
        target->getName().c_str(), attr.c_str(), delta);
  }
  else if (delta < 0)
  {
    battle_message("%s's %s decreased by %d",
        target->getName().c_str(), attr.c_str(), delta);
  }
  else
  {
    battle_message("No effect...");
  }
}

#include <algorithm>

#include "Utility.h"
#include "Message.h"
#include "StatusEffect.h"
#include "Vocabulary.h"
#include "LuaBindings.h"

#include "Attack.h"

namespace
{
  std::string _wrap_in_function(const std::string& formula)
  {
    return std::string("function __calc_damage(a, b)\n return ") + formula + "\nend";
  }
}

int attack(Character* attacker, Character* target, bool guard, Item* weapon, bool& wasCritical)
{
  int damage = calculate_physical_damage(attacker, target, weapon);

  int aSpeed = attacker->computeCurrentAttribute(terms::speed);
  int bSpeed = target->computeCurrentAttribute(terms::speed);

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
    bool critical = attacker->computeCurrentAttribute(terms::luck) >= random_range(0, 1024);
    wasCritical = critical;

    if (critical)
    {
      battle_message("Critical hit!!");
      damage *= 3;
    }
  }

  target->takeDamage(terms::hp, damage);

  target->flash().addDamageText(toString(damage), sf::Color::Red);

  return damage;
}

int calculate_physical_damage(Character* attacker, Character* target, Item* weapon)
{
  float damage = 0;
  float resist = 1.0f;

  if (weapon && weapon->formula.size())
  {
    std::string funcWrap = _wrap_in_function(weapon->formula);

    global_lua_env()->executeLine(funcWrap);
    damage = global_lua_env()->call_function_result<double>("__calc_damage", attacker, target);
  }
  else
  {
    float atk = attacker->computeCurrentAttribute(terms::strength);
    float def = target->computeCurrentAttribute(terms::defense);

    damage = (atk / 2.0f - def / 4.0f) * rand_float(0.8f, 1.2f);
  }

  if (weapon)
  {
    for (auto it = weapon->elements.begin(); it != weapon->elements.end(); ++it)
    {
      resist *= target->getResistance(it->first);
    }
  }

  if ((int)damage <= 0)
  {
    damage = random_range(0, 2);
  }

  damage *= resist;

  return damage;
}

int calculate_physical_damage_item(Character* attacker, Character* target, Item* usedItem)
{
  (void)attacker;

  if (usedItem->itemUseType == ITEM_HEAL_FIXED)
  {
    return -usedItem->attributeGain[terms::hp];
  }
  else if (usedItem->itemUseType == ITEM_DAMAGE || usedItem->itemUseType == ITEM_HEAL)
  {
    float atk = usedItem->attributeGain[terms::strength];
    float def = target->computeCurrentAttribute(terms::defense);

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
    return -usedItem->attributeGain[terms::mp];
  }

  return 0;
}

int calculate_magical_damage(Character* attacker, Character* target, const Spell* spell)
{
  float damage = 0;
  float resistance = target->getResistance(spell->element);

  if (spell->formula.empty())
  {
    float str = !spell->isPhysical ? attacker->computeCurrentAttribute(terms::magic)
                                   : attacker->computeCurrentAttribute(terms::strength);
    float pow = spell->power;
    float def = !spell->isPhysical ? target->computeCurrentAttribute(terms::magdef)
                                   : target->computeCurrentAttribute(terms::defense);

    if (spell->spellType & SPELL_HEAL)
    {
      def = 0;
    }

    float atk = (1.0f + str / 255.0f) * pow;

    damage = (atk / 2.0f - def / 4.0f) * rand_float(0.8f, 1.2f);
  }
  else
  {
    std::string funcWrap = _wrap_in_function(spell->formula);

    global_lua_env()->executeLine(funcWrap);
    damage = global_lua_env()->call_function_result<double>("__calc_damage", attacker, target);
  }

  if ((int)damage <= 0)
  {
    damage = 1;
  }

  damage *= resistance;

  if (spell->spellType & SPELL_HEAL)
  {
    damage = -damage;

    target->flash().addDamageText(toString(-(int)damage), sf::Color::Green);
  }
  else
  {
    target->flash().addDamageText(toString((int)damage), sf::Color::Red);
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

    // Set hp to 0 if the status to cause is "Dead"
    if (status == "Dead")
    {
      target->getAttribute(terms::hp).current = 0;
    }

//    battle_message("%s %s",
//        target->getName().c_str(), get_status_effect(status)->verb.c_str());

    target->flash().addDamageText(status, get_status_effect(status)->color);
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

//    battle_message("%s %s",
//        target->getName().c_str(), get_status_effect(status)->recoverVerb.c_str());

    target->flash().addDamageText(status, sf::Color::Green);
  }
  else
  {
    battle_message("No effect...");
  }
}

void buff(Character* target, const std::string& attr, int buffPower)
{
  int currAttr = target->getAttribute(attr).current;
  int baseAttr = target->getAttribute(attr).max;
  float power = 1.0f + (float)buffPower / 100.0f;
  int newAttr = (float)baseAttr * power;

  int delta = newAttr - baseAttr;

  if (delta > 0)
  {
    // Max positive attribute change is 100% of base attribute.
    if (currAttr >= baseAttr * 2)
    {
      delta = 0;
    }
  }
  else if (delta < 0)
  {
    // Max negative attribute change is 50% of base attribute.
    if (currAttr <= baseAttr / 2)
    {
      delta = 0;
    }
  }

  if (delta > 0)
  {
    target->getAttribute(attr).current += delta;
    target->flash().addDamageText("+" + vocab(attr), sf::Color::Green);
  }
  else if (delta < 0)
  {
    target->getAttribute(attr).current += delta;
    target->flash().addDamageText("-" + vocab(attr), sf::Color::Green);
  }
}

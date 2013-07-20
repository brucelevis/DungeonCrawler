#include "Utility.h"
#include "Message.h"
#include "StatusEffect.h"

#include "Attack.h"

int attack(Character* attacker, Character* target, bool guard, Item* weapon)
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

  target->takeDamage("hp", damage);

  return damage;
}

int calculate_physical_damage(Character* attacker, Character* target, Item* weapon)
{
  float level = attacker->computeCurrentAttribute("level");
  float str = attacker->computeCurrentAttribute("strength");
  float pow = attacker->computeCurrentAttribute("power");
  float def = target->computeCurrentAttribute("defense");

  float resist = 1.0f;

  if (weapon)
  {
    for (auto it = weapon->elements.begin(); it != weapon->elements.end(); ++it)
    {
      resist *= target->getResistance(it->first);
    }
  }

  float damage =
      ((((2.0f * level / 5.0f + 2.0f) *
      str * pow / def) / 50.0f) + 2.0f) *
          1.0f * //stab
          resist * //weak
          (85.0f + (float)random_range(0, 16)) / 100.0f;

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
    int level = attacker->computeCurrentAttribute("level");
    int str = usedItem->attributeGain["strength"];
    int pow = usedItem->attributeGain["power"];
    int def = target->computeCurrentAttribute("defense");

    int damage =
        ((((2 * level / 5 + 2) *
        str * pow / def) / 50) + 2) *
            1 * //stab
            1 * //weak
            (85 + random_range(0, 16)) / 100;

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
  float level = attacker->computeCurrentAttribute("level");
  float str = attacker->computeCurrentAttribute("magic");
  float pow = spell->power;
  float def = target->computeCurrentAttribute("mag.def");

  float resistance = target->getResistance(spell->element);

  if (spell->spellType & SPELL_HEAL)
  {
    def = level;
  }

  float damage =
      ((((2.0f * level / 5.0f + 2.0f) *
      str * pow / def) / 50.0f) + 2.0f) *
          1.0f * //stab
          resistance * //weak
          (85.0f + (float)random_range(0, 16)) / 100.0f;

  if (spell->spellType & SPELL_HEAL)
  {
    damage = -damage;
  }

  return damage;
}

void cause_status(Character* target, const std::string& status, bool forceStatus, int duration)
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
    battle_message("No effect...");
  }
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

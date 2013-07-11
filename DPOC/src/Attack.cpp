#include "Utility.h"
#include "Message.h"
#include "StatusEffect.h"

#include "Attack.h"

int attack(Character* attacker, Character* target, bool guard, Item* weapon)
{
  int damage = calculate_physical_damage(attacker, target, weapon);

  if (guard)
  {
    damage /= 2;
  }

  target->takeDamage("hp", damage);

  return damage;
}

int calculate_physical_damage(Character* attacker, Character* target, Item* weapon)
{
  int level = attacker->computeCurrentAttribute("level");
  int str = attacker->computeCurrentAttribute("strength");
  int pow = attacker->computeCurrentAttribute("power");
  int def = target->computeCurrentAttribute("defense");

  int damage =
      ((((2 * level / 5 + 2) *
      str * pow / def) / 50) + 2) *
          1 * //stab
          1 * //weak
          (85 + random_range(0, 16)) / 100;

  return damage;
}

int calculate_physical_damage_item(Character* attacker, Character* target, Item* usedItem)
{
  if (usedItem->itemUseType == Item::ITEM_HEAL_FIXED)
  {
    return -usedItem->attributeGain["hp"];
  }
  else if (usedItem->itemUseType == Item::ITEM_DAMAGE || usedItem->itemUseType == Item::ITEM_HEAL)
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

    if (usedItem->itemUseType == Item::ITEM_HEAL)
    {
      damage = -damage;
    }

    return damage;
  }
  else if (usedItem->itemUseType == Item::ITEM_RESTORE_MP_FIXED)
  {
    return -usedItem->attributeGain["mp"];
  }

  return 0;
}

int calculate_magical_damage(Character* attacker, Character* target, const Spell* spell)
{
  int level = attacker->computeCurrentAttribute("level");
  int str = attacker->computeCurrentAttribute("magic");
  int pow = spell->power;
  int def = target->computeCurrentAttribute("mag.def");

  int damage =
      ((((2 * level / 5 + 2) *
      str * pow / def) / 50) + 2) *
          1 * //stab
          1 * //weak
          (85 + random_range(0, 16)) / 100;

  if (spell->spellType == SPELL_HEAL)
  {
    damage = -damage;
  }

  return damage;
}

void cause_status(Character* target, const std::string& status)
{
  if (target->getStatus() != "Dead" && !target->hasStatus(status))
  {
    target->afflictStatus(status);

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

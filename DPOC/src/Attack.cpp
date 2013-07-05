#include "Utility.h"
#include "Attack.h"

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

  if (spell->spellType == Spell::SPELL_HEAL)
  {
    damage = -damage;
  }

  return damage;
}

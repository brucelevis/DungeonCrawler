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

int calculate_magical_damage(Character* attacker, Character* target, Spell* spell)
{
  return 0;
}

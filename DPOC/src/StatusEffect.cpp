#include <vector>
#include "StatusEffect.h"

static std::vector<StatusEffect> statusEffects =
{
  {
    "Normal", "",
    0, false,
    StatusEffect::DAMAGE_NONE
  },

  {
    "Dead", "has fallen",
    0, 0, true,
    StatusEffect::DAMAGE_NONE
  },

  {
    "Poison", "is poisoned",
    0, false,
    StatusEffect::DAMAGE_FIXED, 1
  }
};

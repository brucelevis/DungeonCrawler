#include <vector>

#include "logger.h"
#include "Utility.h"
#include "StatusEffect.h"

static std::vector<StatusEffect> statusEffects =
{
  {
    "Normal", "", "",
    sf::Color::White,
    false, 0, false,
    StatusEffect::DAMAGE_NONE
  },

  {
    "Dead", "has fallen!", "comes back to life!",
    sf::Color::Red,
    false, 0, true,
    StatusEffect::DAMAGE_NONE
  },

  {
    "Poison", "is poisoned!", "feels better.",
    sf::Color::Green,
    false, 0, false,
    StatusEffect::DAMAGE_FIXED, "hp", 1,
    "Poison.wav"
  },

  {
    "Paralyze", "is paralyzed!", "can move again.",
    sf::Color::Yellow,
    true, 25, true,
    StatusEffect::DAMAGE_NONE
  }
};


StatusEffect* get_status_effect(const std::string& status)
{
  for (auto it = statusEffects.begin(); it != statusEffects.end(); ++it)
  {
    if (to_lower(it->name) == to_lower(status))
    {
      return &(*it);
    }
  }

  TRACE("No status effect %s defined!", status.c_str());

  return 0;
}

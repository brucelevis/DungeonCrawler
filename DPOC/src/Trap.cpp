#include <algorithm>

#include "Sound.h"
#include "Config.h"
#include "Game.h"
#include "Player.h"
#include "PlayerCharacter.h"
#include "Message.h"

#include "Vocabulary.h"
#include "Utility.h"

#include "LuaBindings.h"
#include "Trap.h"

Trap::Trap()
 : x(0),
   y(0),
   m_luckToBeat(0)
{
}

Trap::Trap(const std::string& type, int luck, int _x, int _y)
 : x(_x),
   y(_y),
   m_type(type),
   m_luckToBeat(luck)
{

}

PlayerCharacter* Trap::triggerTrap() const
{
  std::vector<PlayerCharacter*> party = get_player()->getParty();
  std::random_shuffle(party.begin(), party.end());

  PlayerCharacter* detector = 0;

  for (auto it = party.begin(); it != party.end(); ++it)
  {
    if (check_vs_luck((*it)->computeCurrentAttribute(terms::luck), m_luckToBeat))
    {
      detector = *it;
      break;
    }
  }

  return detector;
}

void Trap::checkTrap() const
{
  PlayerCharacter* detector = triggerTrap();

  if (detector)
  {
    play_sound(config::get("SOUND_SUCCESS"));
    show_message("%s found and disarmed a trap!", detector->getName().c_str());
  }
  else
  {
    show_message("The party stumbled into a %s trap!", m_type.c_str());
    play_sound(config::get("SOUND_TRAP"));

    applyTrap(get_player()->getParty());
  }
}

void Trap::applyTrap(const std::vector<PlayerCharacter*>& party) const
{
  if (m_type == "poison")
  {
    for (auto it = party.begin(); it != party.end(); ++it)
    {
      if (!check_vs_luck((*it)->computeCurrentAttribute(terms::luck), m_luckToBeat))
      {
        (*it)->afflictStatus("Poison", -1);

        show_message("%s was poisoned!", (*it)->getName().c_str());
      }
    }
    //run_lua_script("Scripts/PoisonTrap.lua");
  }
}

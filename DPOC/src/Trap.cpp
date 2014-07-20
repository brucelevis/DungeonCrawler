#include <algorithm>

#include "Game.h"
#include "Player.h"
#include "PlayerCharacter.h"
#include "Message.h"

#include "Utility.h"

#include "Trap.h"

Trap::Trap(const std::string& type, int luck, int _x, int _y)
 : x(_x),
   y(_y),
   m_type(type),
   m_luckToBeat(luck)
{

}

void Trap::checkTrap() const
{
  std::vector<PlayerCharacter*> party = get_player()->getParty();
  std::random_shuffle(party.begin(), party.end());

  PlayerCharacter* detector = 0;

  for (auto it = party.begin(); it != party.end(); ++it)
  {
    if (check_vs_luck((*it)->computeCurrentAttribute("luck"), m_luckToBeat))
    {
      detector = *it;
      break;
    }
  }

  if (detector)
  {
    show_message("%s found a trap!", detector->getName().c_str());
  }
  else
  {
    applyTrap(party);
  }

}

void Trap::applyTrap(const std::vector<PlayerCharacter*>& party) const
{
  show_message("The party stumbled into a %s trap!", m_type.c_str());

  if (m_type == "poison")
  {
    for (auto it = party.begin(); it != party.end(); ++it)
    {
      if (!check_vs_luck((*it)->computeCurrentAttribute("luck"), m_luckToBeat))
      {
        (*it)->afflictStatus("Poison", -1);

        show_message("%s was poisoned!", (*it)->getName().c_str());
      }
    }
  }
}

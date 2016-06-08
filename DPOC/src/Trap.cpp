#include <algorithm>

#include "Sound.h"
#include "Config.h"
#include "Game.h"
#include "Player.h"
#include "PlayerCharacter.h"
#include "Message.h"

#include "Vocabulary.h"
#include "Utility.h"

#include "Trap.h"

Trap::Trap()
 : x(0),
   y(0),
   m_difficulty(0)
{
}

Trap::Trap(const std::string& type, int difficulty, int _x, int _y)
 : x(_x),
   y(_y),
   m_type(type),
   m_difficulty(difficulty)
{

}

PlayerCharacter* Trap::triggerTrap() const
{
  std::vector<PlayerCharacter*> party = get_player()->getParty();
  std::random_shuffle(party.begin(), party.end());

  PlayerCharacter* detector = 0;

  for (auto& character : party)
  {
    int totalSkill = character->computeCurrentAttribute(terms::luck) + character->getBaseAttribute(terms::searching);

    if (random_range(0, totalSkill) >= m_difficulty)
    {
      totalSkill = character->computeCurrentAttribute(terms::luck) + character->getBaseAttribute(terms::mechanics);

      if (random_range(0, totalSkill) >= m_difficulty)
      {
        detector = character;
        break;
      }
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
  for (auto& character : party)
  {
    int totalSkill = character->computeCurrentAttribute(terms::speed) / 2 +
        character->computeCurrentAttribute(terms::luck) / 2 +
        character->getBaseAttribute(terms::evasion);

    if (random_range(0, totalSkill) < m_difficulty)
    {
      if (m_type == "poison")
      {
        character->afflictStatus("Poison", -1);

        show_message("%s was poisoned!", character->getName().c_str());
      }
    }
  }
}

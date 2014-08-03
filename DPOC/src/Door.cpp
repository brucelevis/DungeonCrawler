#include "Persistent.h"
#include "Sound.h"
#include "Player.h"
#include "PlayerCharacter.h"
#include "Message.h"
#include "Config.h"

#include "Door.h"

Door::Door(const std::string& key)
 : m_keyRequired(key),
   m_isTrapped(false)
{

}

Door::Door(const std::string& key, const std::string& trapType, int luckToBeat)
 : m_keyRequired(key),
   m_isTrapped(true),
   m_trap(trapType, luckToBeat, 0, 0)
{

}

void Door::update()
{
  if (isVisible() && isOpen())
  {
    open();
  }
}

void Door::interact(const Entity*)
{
  std::string key = getTag();

  if (!isOpen())
  {
    if ( (m_keyRequired.size() && get_player()->getItem(m_keyRequired)) ||
         (m_keyRequired.empty()) )
    {
      if (m_keyRequired.size())
      {
        show_message("You enter the %s in the key hole and push the door open.", m_keyRequired.c_str());
        get_player()->removeItemFromInventory(m_keyRequired, 1);
      }

      if (m_isTrapped)
      {
        PlayerCharacter* character = m_trap.triggerTrap();

        if (character)
        {
          show_message("Upon opening the door, %s spots and disarms a trap!", character->getName().c_str());
        }
        else
        {
          if (config::isSet("SOUND_TRAP"))
          {
            play_sound(config::get("SOUND_TRAP"));
          }

          show_message("Upon opening the door, you trigger a trap!");
          m_trap.applyTrap(get_player()->getParty());
        }
      }

      if (config::isSet("SOUND_DOOR"))
      {
        play_sound(config::get("SOUND_DOOR"));
      }

      open();
    }
    else
    {
      show_message("This door is locked. You will need some kind of key to bybass it.");
    }
  }
}

void Door::open()
{
  setIsVisible(false);
  setWalkThrough(true);

  Persistent<int>::instance().set(getTag(), true);
}

bool Door::isOpen() const
{
  return Persistent<int>::instance().isSet(getTag());
}

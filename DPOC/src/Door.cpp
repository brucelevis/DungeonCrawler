#include "Persistent.h"
#include "Sound.h"
#include "Player.h"
#include "PlayerCharacter.h"
#include "Message.h"
#include "Config.h"

#include "Door.h"

namespace
{
  const int MAX_CLOSE_COUNTER = 300;
  const float DOOR_SPEED = 0.05f;
}

Door::Door(const std::string& name, const std::string& key)
 : Entity(name),
   m_keyRequired(key),
   m_isTrapped(false),
   m_state(Door_Closed),
   m_openingCount(1),
   m_closeCounter(MAX_CLOSE_COUNTER)
{

}

Door::Door(const std::string& name, const std::string& key, const std::string& trapType, int luckToBeat)
 : Entity(name),
   m_keyRequired(key),
   m_isTrapped(true),
   m_trap(trapType, luckToBeat, 0, 0),
   m_state(Door_Closed),
   m_openingCount(1),
   m_closeCounter(MAX_CLOSE_COUNTER)
{

}

void Door::update()
{
  if (isVisible() && isOpen())
  {
    openFinished();
  }
  else if (isOpening())
  {
    m_openingCount -= DOOR_SPEED;

    if (m_openingCount <= 0)
    {
      openFinished();
    }
  }
  else if (isOpen() && m_keyRequired.empty())
  {
    m_closeCounter--;
    if (m_closeCounter <= 0)
    {
      close();
    }
  }
  else if (m_state == Door_Closing)
  {
    m_openingCount += DOOR_SPEED;

    if (m_openingCount >= 1)
    {
      closeFinished();
    }
  }
}

void Door::interact(const Entity*)
{
  std::string key = getTag();

  if (!isOpen() && !isOpening())
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

bool Door::isSeeThrough() const
{
  return m_state == Door_Opening || m_state == Door_Closing || m_state == Door_Open;
}

bool Door::isOpening() const
{
  return m_state == Door_Opening;
}

float Door::getOpeningCount() const
{
  if (m_state == Door_Open)
  {
    return 0.05f;
  }

  return m_openingCount;
}

void Door::open()
{
  m_state = Door_Opening;
  m_openingCount = 1;
}

void Door::openFinished()
{
  m_state = Door_Open;
  m_openingCount = 0;
  m_closeCounter = MAX_CLOSE_COUNTER;

  setIsVisible(false);
  setWalkThrough(true);

  Persistent<int>::instance().set(getTag(), true);
}

bool Door::isOpen() const
{
  return Persistent<int>::instance().isSet(getTag()) &&
         Persistent<int>::instance().get(getTag());
}

void Door::close()
{
  m_state = Door_Closing;
  m_openingCount = 0.05f;

  setIsVisible(true);
  Persistent<int>::instance().set(getTag(), false);
}

void Door::closeFinished()
{
  m_state = Door_Closed;
  m_openingCount = 1;
  m_closeCounter = MAX_CLOSE_COUNTER;

  setWalkThrough(false);
}

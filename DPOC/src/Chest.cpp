#include "Utility.h"
#include "Item.h"
#include "Player.h"
#include "PlayerCharacter.h"
#include "Message.h"
#include "Sound.h"
#include "Config.h"
#include "Persistent.h"
#include "Sprite.h"
#include "Vocabulary.h"
#include "Chest.h"

Chest::Chest(const std::string& name, const std::vector<std::string> items)
 : Entity(name),
   m_items(items),
   m_spriteChanged(false),
   m_isTrapped(false)
{
}

Chest::Chest(const std::string& name, const std::vector<std::string> items, const std::string& trapType, int luckToBeat)
 : Entity(name),
   m_items(items),
   m_spriteChanged(false),
   m_isTrapped(true),
   m_trap(trapType, luckToBeat, 0, 0)
{
}

void Chest::update()
{
  if (!m_spriteChanged && Persistent::instance().isSet(getTag()))
  {
    changeSprite();
  }
}

void Chest::interact(const Entity* interactor)
{
  (void)interactor;

  std::string key = getTag();

  if (!Persistent::instance().isSet(key))
  {
    changeSprite();
    Persistent::instance().set(key, 1);

    if (!m_isTrapped)
    {
      play_sound(config::get("SOUND_CHEST"));

      show_message("You carefully open the lid of the chest, finding %s!",
          getItemsString().c_str());
    }
    else
    {
      PlayerCharacter* character = m_trap.triggerTrap();
      if (character)
      {
        play_sound(config::get("SOUND_CHEST"));

        show_message("Upon opening the chest, %s spots and disarms a trap! Inside you find %s!",
            character->getName().c_str(), getItemsString().c_str());
      }
      else
      {
        play_sound(config::get("SOUND_TRAP"));

        show_message("Upon opening the chest, you trigger a trap!");
        m_trap.applyTrap(get_player()->getParty());
        show_message("For your troubles, you find %s!", getItemsString().c_str());
      }
    }
    for (auto it = m_items.begin(); it != m_items.end(); ++it)
    {
      if (isdigit(it->at(0)) && it->find(terms::gold) != std::string::npos)
      {
        std::vector<std::string> golds = split_string(*it, ' ');
        int amount = fromString<int>(golds[0]);
        get_player()->gainGold(amount);
      }
      else
      {
        get_player()->addItemToInventory(*it, 1);
      }
    }
  }
  else
  {
    show_message("Already looted...");
  }
}

void Chest::changeSprite()
{
  TileSprite* tileSprite = dynamic_cast<TileSprite*>(sprite());
  tileSprite->setTileNum(tileSprite->getTileNum() + 1);

  m_spriteChanged = true;
}

std::string Chest::getItemsString() const
{
  std::string things;

  if (m_items.size() == 1)
  {
    things = m_items.front();
  }
  else if (m_items.empty())
  {
    things = "... nothing";
  }
  else
  {
    for (size_t i = 0; i < m_items.size(); i++)
    {
      std::string itemName = m_items[i];

      if (m_items[i] == terms::gold)
      {
        itemName = vocab(terms::gold);
      }

      things += itemName;
      if (i == m_items.size() - 2)
      {
        things += " and ";
      }
      else if (i < m_items.size() - 1)
      {
        things += ", ";
      }
    }
  }

  return things;
}

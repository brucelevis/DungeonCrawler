#include "Utility.h"
#include "Item.h"
#include "Player.h"
#include "Message.h"
#include "Sound.h"
#include "Config.h"
#include "Persistent.h"
#include "Sprite.h"
#include "Chest.h"

Chest::Chest(const std::vector<std::string> items)
 : m_items(items),
   m_spriteChanged(false)
{

}

void Chest::update()
{
  if (!m_spriteChanged && Persistent<int>::instance().isSet(getTag()))
  {
    changeSprite();
  }
}

void Chest::interact(const Entity* interactor)
{
  std::string key = getTag();

  if (!Persistent<int>::instance().isSet(key))
  {
    play_sound(config::get("SOUND_CHEST"));

    changeSprite();
    Persistent<int>::instance().set(key, 1);

    show_message("You carefully open the lid of the chest, finding %s!",
        getItemsString().c_str());
    for (auto it = m_items.begin(); it != m_items.end(); ++it)
    {
      if (isdigit(it->at(0)) && it->find("gold") != std::string::npos)
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
      things += m_items[i];
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

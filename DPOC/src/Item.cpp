#include <vector>
#include <stdexcept>

#include "logger.h"
#include "Character.h"
#include "Utility.h"
#include "Message.h"
#include "Item.h"

static std::vector<Item> itemDefinitions =
{
  {
    "Herb", "A medicin herb",
    25,
    ITEM_USE,
    TARGET_SINGLE_ALLY,
    { { "hp", 25 } }
  },

  {
    "Rusty Knife", "An old rusty knife",
    10,
    ITEM_WEAPON,
    TARGET_NONE,
    { { "power", 4 } },
    "Effect_Slash"
  },

  {
    "Wood Shield", "A wooden shield",
    25,
    ITEM_SHIELD,
    TARGET_NONE,
    { { "defense", 4 } }
  }
};

Item create_item(const std::string& name, int stackSize)
{
  for (auto it = itemDefinitions.begin(); it != itemDefinitions.end(); ++it)
  {
    if (to_lower(it->name) == to_lower(name))
    {
      Item itemCopy = *it;
      itemCopy.stackSize = stackSize;
      return itemCopy;
    }
  }

  return Item();
}

Item& item_ref(const std::string& name)
{
  for (auto it = itemDefinitions.begin(); it != itemDefinitions.end(); ++it)
  {
    if (to_lower(it->name) == to_lower(name))
      return *it;
  }

  TRACE("No item %s defined!", name.c_str());

  throw std::runtime_error("No item " + name + " defined!");
}

void use_item(Item* item, Character* user, Character* target)
{
  Message::instance().show(user->getName() + " uses " + item->name + " on " + target->getName() + "!");
}

std::string equip_type_string(ItemType itemType)
{
  if (itemType == ITEM_WEAPON) return "Weapon";
  else if (itemType == ITEM_SHIELD) return "Shield";
  else if (itemType == ITEM_ARMOR) return "Armour";
  else if (itemType == ITEM_HELMET) return "Helmet";
  else if (itemType == ITEM_MISC) return "Others";
  return "";
}

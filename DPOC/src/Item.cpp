#include <vector>

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
    { { "hp", 25 } }
  },

  {
    "Rusty Knife", "An old rusty knife",
    10,
    ITEM_WEAPON,
    { { "power", 4 } }
  },

  {
    "Wood Shield", "A wooden shield",
    25,
    ITEM_SHIELD,
    { { "defense", 4 } }
  }
};

Item create_item(const std::string& name, int stackSize)
{
  for (auto it = itemDefinitions.begin(); it != itemDefinitions.end(); ++it)
  {
    if (toLower(it->name) == toLower(name))
    {
      Item itemCopy = *it;
      itemCopy.stackSize = stackSize;
      return itemCopy;
    }
  }

  return Item();
}

void use_item(Item* item, Character* user, Character* target)
{
  Message::instance().show(user->getName() + " uses " + item->name + " on " + target->getName() + "!");
}

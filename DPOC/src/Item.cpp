#include <vector>
#include "Item.h"

static std::vector<Item> itemDefinitions =
{
  {
    "Herb", "A medicin herb",
    25,
    ITEM_USE
  },

  {
    "Rusty Knife", "An old rusty knife",
    10,
    ITEM_WEAPON
  },

  {
    "Wood Shield", "A wooden shield",
    25,
    ITEM_SHIELD
  }
};

Item create_item(const std::string& name, int stackSize)
{
  for (auto it = itemDefinitions.begin(); it != itemDefinitions.end(); ++it)
  {
    if (it->name == name)
    {
      Item itemCopy = *it;
      itemCopy.stackSize = stackSize;
      return itemCopy;
    }
  }

  return Item();
}

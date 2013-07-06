#include <vector>
#include <stdexcept>

#include "Attack.h"
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
    {
      { "hp", 25 }
    },
    "",
    Item::ITEM_HEAL_FIXED
  },

  {
    "Ether", "Delicious",
    150,
    ITEM_USE,
    TARGET_SINGLE_ALLY,
    {
      { "mp", 25 }
    },
    "",
    Item::ITEM_RESTORE_MP_FIXED
  },

  {
    "Antidote", "Cures poison",
    50,
    ITEM_USE,
    TARGET_SINGLE_ALLY,
    {},
    "",
    Item::ITEM_REMOVE_STATUS,
    "Poison"
  },

  {
    "Firebomb", "Explodes in flames",
    100,
    ITEM_USE,
    TARGET_SINGLE_ENEMY,
    {
      { "strength", 120 },
      { "power", 120 }
    },
    "Effect_Flame",
    Item::ITEM_DAMAGE
  },

  {
    "Steroids", "Makes you mighty",
    500,
    ITEM_USE,
    TARGET_SINGLE_ALLY,
    {
      { "strength", 120 },
      { "power", 120 }
    },
    "",
    Item::ITEM_BUFF
  },

  {
    "Rusty Knife", "An old rusty knife",
    10,
    ITEM_WEAPON,
    TARGET_NONE,
    {
      { "power", 4 }
    },
    "Effect_Slash"
  },

  {
    "Wood Shield", "A wooden shield",
    25,
    ITEM_SHIELD,
    TARGET_NONE,
    {
      { "defense", 4 }
    }
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

int use_item(Item* item, Character* user, Character* target)
{
  int damage = calculate_physical_damage_item(user, target, item);

  for (auto it = item->attributeGain.begin(); it != item->attributeGain.end(); ++it)
  {
    if (item->itemUseType == Item::ITEM_HEAL || item->itemUseType == Item::ITEM_HEAL_FIXED ||
        item->itemUseType == Item::ITEM_DAMAGE)
    {
      target->takeDamage(it->first, damage);
    }
    else if (item->itemUseType == Item::ITEM_RESTORE_MP || item->itemUseType == Item::ITEM_RESTORE_MP_FIXED)
    {
      target->takeDamage(it->first, damage);

      battle_message("%s's MP restored by %d!",
          target->getName().c_str(), damage);

      damage = 0;
    }
    else if (item->itemUseType == Item::ITEM_BUFF)
    {
      buff(target, it->first, it->second);
    }
  }

  if (item->itemUseType == Item::ITEM_REMOVE_STATUS)
  {
    cure_status(target, item->status);
  }
  else if (item->itemUseType == Item::ITEM_CAUSE_STATUS)
  {
    cause_status(target, item->status);
  }

  return damage;
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

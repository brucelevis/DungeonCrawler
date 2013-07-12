#ifndef ITEM_H
#define ITEM_H

#include <string>
#include <map>

#include "Target.h"

class Character;

enum ItemType
{
  ITEM_USE,
  ITEM_WEAPON,
  ITEM_SHIELD,
  ITEM_ARMOR,
  ITEM_HELMET,
  ITEM_MISC
};

enum ItemUseType
{
  ITEM_HEAL,
  ITEM_HEAL_FIXED,
  ITEM_RESTORE_MP,
  ITEM_RESTORE_MP_FIXED,
  ITEM_DAMAGE,
  ITEM_BUFF,
  ITEM_REMOVE_STATUS,
  ITEM_CAUSE_STATUS,
  ITEM_CUSTOM
};

struct Item
{
  std::string name;
  std::string description;
  int cost;
  ItemType type;

  Target target;

  std::map<std::string, int> attributeGain;

  std::string effect;

  ItemUseType itemUseType;

  std::string status;

  // Put members that should not be initialized after this comment.

  int stackSize;
};

Item create_item(const std::string& name, int stackSize = 1);
Item& item_ref(const std::string& name);
int use_item(Item* item, Character* user, Character* target);

std::string equip_type_string(ItemType itemType);

#endif

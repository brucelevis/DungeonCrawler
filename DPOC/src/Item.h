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

struct Item
{
  std::string name;
  std::string description;
  int cost;
  ItemType type;

  Target target;

  std::map<std::string, int> attributeGain;

  // Put members that should not be initialized after this comment.

  int stackSize;
};

Item create_item(const std::string& name, int stackSize = 1);
Item& item_ref(const std::string& name);
void use_item(Item* item, Character* user, Character* target);

std::string equip_type_string(ItemType itemType);

#endif

#ifndef ITEM_H
#define ITEM_H

#include <string>
#include <map>

class Character;

enum ItemType
{
  ITEM_USE,
  ITEM_WEAPON,
  ITEM_SHIELD,
  ITEM_ARMOR,
  ITEM_HELMET
};

struct Item
{
  std::string name;
  std::string description;
  int cost;
  ItemType type;

  std::map<std::string, int> attributeGain;

  // Put members that should not be initialized after this comment.

  int stackSize;
};

Item create_item(const std::string& name, int stackSize = 1);
void use_item(Item* item, Character* user, Character* target);

#endif

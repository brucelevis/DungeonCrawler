#ifndef ITEM_H
#define ITEM_H

#include <string>

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

  int stackSize;
};

Item create_item(const std::string& name, int stackSize = 1);

#endif

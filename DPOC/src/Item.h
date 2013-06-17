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

struct ItemDef
{
  std::string name;
  int cost;
  ItemType type;
};

class Item
{
public:
  Item(const std::string& name);

  void merge(const Item& rhs);

  std::string getName() const { return m_definition.name; }
public:
  int stack;
private:
  ItemDef m_definition;
};

#endif

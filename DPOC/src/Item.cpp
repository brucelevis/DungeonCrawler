#include <vector>
#include "Item.h"

static std::vector<ItemDef> itemDefinitions =
{
  {
    "Herb",
    25,
    ITEM_USE
  }
};

Item::Item(const std::string& name)
 : stack(1)
{
  for (auto it = itemDefinitions.begin(); it != itemDefinitions.end(); ++it)
  {
    if (it->name == name)
    {
      m_definition = *it;
    }
  }
}

void Item::merge(const Item& rhs)
{
  if (rhs.m_definition.name == m_definition.name)
  {
    stack += rhs.stack;
  }
}

#include "Item.h"
#include "Player.h"
#include "Menu_EquipItemMenu.h"

EquipItemMenu::EquipItemMenu(PlayerCharacter* character, const Callback& callback, int x, int y, int width, int height)
  : ItemMenu(callback, x, y, width, height),
    m_character(character)
{
}

void EquipItemMenu::refresh()
{
  m_items.clear();

  const std::vector<Item>& items = get_player()->getInventory();

  m_items.push_back(removeEquipmentString());

  for (auto it = items.begin(); it != items.end(); ++it)
  {
    if (equip_type_string(it->type) == m_equipmentType && m_character->canEquip(*it))
    {
      m_items.push_back(it->name);
    }
  }

  fixRange();
}

void EquipItemMenu::setEquipmentType(const std::string& equipmentType)
{
  m_equipmentType = equipmentType;
}

std::string EquipItemMenu::removeEquipmentString()
{
  return "* Remove *";
}

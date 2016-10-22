#ifndef MENU_EQUIPITEMMENU_H_
#define MENU_EQUIPITEMMENU_H_

#include <string>

#include "PlayerCharacter.h"
#include "Menu_ItemMenu.h"

class EquipItemMenu : public ItemMenu
{
public:
  EquipItemMenu(PlayerCharacter* character, const Callback& callback, int x, int y, int width, int height);

  void refresh() override;

  bool handleInput(sf::Keyboard::Key key) override;

  void setEquipmentType(const std::string& equipmentType);

  static std::string removeEquipmentString();
private:
  PlayerCharacter* m_character;
  std::string m_equipmentType;
};

#endif /* MENU_EQUIPITEMMENU_H_ */

#ifndef MENU_EQUIPMENU_H_
#define MENU_EQUIPMENU_H_

#include <string>
#include <vector>

#include "Range.h"
#include "GuiWidget.h"
#include "PlayerClass.h"

class EquipItemMenu;

class EquipMenu : public GuiWidget
{
public:
  EquipMenu(PlayerCharacter* character, int x, int y);

  void start() override;
  bool handleInput(sf::Keyboard::Key key) override;
  void draw(sf::RenderTarget& target) override;
private:
  std::string getCurrentEquipType() const;
  void handleSelectEquipmentCallback(const std::string& itemName);

  void doEquip(const std::string& itemName);
  void doUnEquip();

  void drawDeltas(sf::RenderTarget& target);
  void refreshItemMenu();
private:
  int m_x, m_y;
  PlayerCharacter* m_character;
  std::vector<std::string> m_equipNames;
  Range m_range;
  EquipItemMenu* m_equipItemMenu;
};

#endif /* MENU_EQUIPMENU_H_ */

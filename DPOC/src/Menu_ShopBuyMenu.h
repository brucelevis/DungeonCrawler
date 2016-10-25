#ifndef MENU_SHOPBUYMENU_H_
#define MENU_SHOPBUYMENU_H_

#include <string>
#include <vector>

#include "GuiWidget.h"
#include "MenuPresenter.h"

class PlayerCharacter;

class ShopBuyMenu : public GuiWidget
{
public:
  ShopBuyMenu(const std::vector<std::string>& inventory);

  int getWidth() const;
  int getHeight() const;

  bool handleInput(sf::Keyboard::Key key) override;
  void draw(sf::RenderTarget& target) override;
private:
  void handleConfirm();
  void handleEscape();

  void drawDeltas(sf::RenderTarget& target, PlayerCharacter* character, const std::string& itemName, int x, int y);
private:
  std::vector<std::string> m_inventory;
  MenuPresenter m_presenter;
};

#endif /* MENU_SHOPBUYMENU_H_ */

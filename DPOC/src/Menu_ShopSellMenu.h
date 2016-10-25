#ifndef MENU_SHOPSELLMENU_H_
#define MENU_SHOPSELLMENU_H_

#include <vector>
#include <string>

#include "GuiWidget.h"
#include "MenuPresenter.h"

class ShopSellMenu : public GuiWidget
{
public:
  ShopSellMenu();

  void moveArrow(Direction dir);

  int getWidth() const;
  int getHeight() const;

  bool handleInput(sf::Keyboard::Key key) override;
  void draw(sf::RenderTarget& target) override;
private:
  void handleConfirm();
  void handleEscape();
  void refresh();
private:
  MenuPresenter m_presenter;
};

#endif /* MENU_SHOPSELLMENU_H_ */

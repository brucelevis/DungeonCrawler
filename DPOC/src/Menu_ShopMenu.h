#ifndef MENU_SHOPMENU_H_
#define MENU_SHOPMENU_H_

#include <vector>
#include <string>
#include <functional>

#include "GuiWidget.h"
#include "MenuPresenter.h"

class ShopMenu : public GuiWidget
{
public:
  using CloseCallback = std::function<void()>;

  ShopMenu(const std::vector<std::string>& inventory, const CloseCallback& callback);

  bool handleInput(sf::Keyboard::Key key) override;
  void draw(sf::RenderTarget& target) override;
private:
  void handleConfirm();
  void handleEscape();
private:
  std::vector<std::string> m_inventory;
  MenuPresenter m_presenter;
  CloseCallback m_closeCallback;
};

#endif /* MENU_SHOPMENU_H_ */

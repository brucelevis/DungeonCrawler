#ifndef MENU_SHOPCONFIRMMENU_H_
#define MENU_SHOPCONFIRMMENU_H_

#include <string>

#include "GuiWidget.h"
#include "MenuPresenter.h"

class ShopConfirmMenu : public GuiWidget
{
public:
  enum BuyOrSell
  {
    BUY, SELL
  };

  ShopConfirmMenu(BuyOrSell type, const std::string& itemName);

  bool handleInput(sf::Keyboard::Key key) override;
  void draw(sf::RenderTarget& target) override;

  int getWidth() const
  {
    return 104;
  }
private:
  void handleConfirm();
  int getSum() const;
private:
  int m_quantity;
  BuyOrSell m_type;
  std::string m_itemName;
  MenuPresenter m_presenter;
};

#endif /* MENU_SHOPCONFIRMMENU_H_ */

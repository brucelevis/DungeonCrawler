#include "Config.h"
#include "Sound.h"
#include "Frame.h"
#include "draw_text.h"
#include "GuiStack.h"
#include "Player.h"

#include "Menu_ShopBuyMenu.h"
#include "Menu_ShopSellMenu.h"
#include "Menu_ShopMenu.h"

ShopMenu::ShopMenu(const std::vector<std::string>& inventory, const CloseCallback& callback)
 : m_inventory(inventory),
   m_presenter(MenuPresenter::STYLE_FRAME),
   m_closeCallback(callback)
{
  m_presenter.addEntry("Buy");
  m_presenter.addEntry("Sell");
  m_presenter.addEntry("Leave");
}

bool ShopMenu::handleInput(sf::Keyboard::Key key)
{
  switch (key)
  {
  case sf::Keyboard::Up:
    m_presenter.scrollUp();
    break;
  case sf::Keyboard::Down:
    m_presenter.scrollDown();
    break;
  case sf::Keyboard::Space:
  case sf::Keyboard::Return:
    handleConfirm();
    break;
  case sf::Keyboard::Escape:
    handleEscape();
    break;
  default:
    break;
  }

  return true;
}

void ShopMenu::draw(sf::RenderTarget& target)
{
  draw_frame(target, 0, 0, config::GAME_RES_X, config::GAME_RES_Y);
  draw_frame(target, 0, 0, config::GAME_RES_X, 24);

  m_presenter.draw(target, 0, config::GAME_RES_Y - m_presenter.getHeight(), this);

  draw_text_bmp(target, 8, 8, "What can I do you for?");
}

void ShopMenu::handleConfirm()
{
  std::string choice = m_presenter.getSelectedOption().entryName;

  if (choice == "Buy")
  {
    getGuiStack()->addWidget<ShopBuyMenu>(m_inventory);
  }
  else if (choice == "Sell")
  {
    if (!get_player()->getInventory().empty())
    {
      getGuiStack()->addWidget<ShopSellMenu>();
    }
    else
    {
      play_sound(config::get("SOUND_CANCEL"));
    }
  }
  else if (choice == "Leave")
  {
    handleEscape();
  }
}

void ShopMenu::handleEscape()
{
  m_closeCallback();
  close();
}

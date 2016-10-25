#include "Frame.h"
#include "draw_text.h"
#include "Config.h"
#include "Utility.h"
#include "Sound.h"
#include "Vocabulary.h"

#include "PlayerCharacter.h"
#include "Player.h"
#include "Entity.h"
#include "Sprite.h"
#include "Direction.h"

#include "Item.h"

#include "Shop.h"

ShopMenu::ShopMenu(const std::vector<std::string>& inventory)
 : m_buyMenu(0),
   m_sellMenu(0),
   m_inventory(inventory)
{
  addEntry("Buy");
  addEntry("Sell");
  addEntry("Leave");

  setVisible(true);
}

ShopMenu::~ShopMenu()
{
  delete m_buyMenu;
  delete m_sellMenu;
}

void ShopMenu::moveArrow(Direction dir)
{
  if (m_buyMenu)
  {
    m_buyMenu->moveArrow(dir);
  }
  else if (m_sellMenu)
  {
    m_sellMenu->moveArrow(dir);
  }
  else
  {
    Menu::moveArrow(dir);
  }
}

void ShopMenu::handleConfirm()
{
  std::string choice = getCurrentMenuChoice();

  if (m_buyMenu)
  {
    m_buyMenu->handleConfirm();
  }
  else if (m_sellMenu)
  {
    m_sellMenu->handleConfirm();

    // If the player runs out of items.
    if (!m_sellMenu->isVisible())
    {
      delete m_sellMenu;
      m_sellMenu = 0;
    }
  }
  else
  {
    if (choice == "Buy")
    {
      m_buyMenu = new ShopBuyMenu(m_inventory);
      m_buyMenu->setVisible(true);
    }
    else if (choice == "Sell")
    {
      if (!get_player()->getInventory().empty())
      {
        m_sellMenu = new ShopSellMenu;
        m_sellMenu->setVisible(true);
      }
      else
      {
        play_sound(config::get("SOUND_CANCEL"));
      }
    }
    else if (choice == "Leave")
    {
      setVisible(false);
    }
  }
}

void ShopMenu::handleEscape()
{
  std::string choice = getCurrentMenuChoice();

  if (m_buyMenu)
  {
    m_buyMenu->handleEscape();
    if (!m_buyMenu->isVisible())
    {
      delete m_buyMenu;
      m_buyMenu = 0;
    }
  }
  else if (m_sellMenu)
  {
    m_sellMenu->handleEscape();
    if (!m_sellMenu->isVisible())
    {
      delete m_sellMenu;
      m_sellMenu = 0;
    }
  }
  else
  {
    setVisible(false);
  }
}

void ShopMenu::draw(sf::RenderTarget& target, int, int)
{
  draw_frame(target, 0, 0, config::GAME_RES_X, config::GAME_RES_Y);
  draw_frame(target, 0, 0, config::GAME_RES_X, 24);

  if (!m_buyMenu && !m_sellMenu)
  {
    Menu::draw(target, 0, config::GAME_RES_Y - getHeight());

    draw_text_bmp(target, 8, 8, "What can I do you for?");
  }
  else if (m_buyMenu)
  {
    m_buyMenu->draw(target, 0, 24);
  }
  else if (m_sellMenu)
  {
    m_sellMenu->draw(target, 0, 24);
  }
}

///////////////////////////////////////////////////////////////////////////////

Shop::Shop(const std::vector<std::string>& items)
 : m_menu(items)
{

}

void Shop::update()
{
  if (!m_menu.isVisible())
  {
    close();
  }
}

void Shop::draw(sf::RenderTarget& target)
{
  m_menu.draw(target, 0, 0);
}

void Shop::handleEvent(sf::Event& event)
{
  switch (event.type)
  {
  case sf::Event::KeyPressed:
    handleKeyPress(event.key.code);

    break;
  default:
    break;
  }
}

void Shop::handleKeyPress(sf::Keyboard::Key key)
{
  if (key == sf::Keyboard::Space)
  {
    m_menu.handleConfirm();
  }
  else if (key == sf::Keyboard::Escape)
  {
    m_menu.handleEscape();
  }
  else if (key == sf::Keyboard::Down) m_menu.moveArrow(DIR_DOWN);
  else if (key == sf::Keyboard::Up) m_menu.moveArrow(DIR_UP);
  else if (key == sf::Keyboard::Right) m_menu.moveArrow(DIR_RIGHT);
  else if (key == sf::Keyboard::Left) m_menu.moveArrow(DIR_LEFT);
}

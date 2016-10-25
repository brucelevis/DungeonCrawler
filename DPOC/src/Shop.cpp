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

ShopSellMenu::ShopSellMenu()
 : m_confirmMenu(0)
{
  refresh();
}

ShopSellMenu::~ShopSellMenu()
{
  delete m_confirmMenu;
}

void ShopSellMenu::refresh()
{
  clear();

  setMaxVisible(7);

  for (auto it = get_player()->getInventory().begin(); it != get_player()->getInventory().end(); ++it)
  {
    std::string itemName = it->name;

    std::string stack = toString(it->stackSize);
    while (stack.size() < 2)
      stack += ' ';

    std::string entry = stack + " " + itemName;
    addEntry(entry);
  }

  if (get_player()->getInventory().empty())
  {
    setVisible(false);
  }
  else
  {
    // Fix when selling last entry.
    if (getCurrentChoiceIndex() >= getNumberOfChoice())
    {
      setCurrentChoice(getNumberOfChoice() - 1);
    }
  }
}

int ShopSellMenu::getWidth() const
{
  return config::GAME_RES_X;
}

int ShopSellMenu::getHeight() const
{
  return 8 * 12;
}

void ShopSellMenu::handleConfirm()
{
  if (m_confirmMenu)
  {
    m_confirmMenu->handleConfirm();
    if (!m_confirmMenu->isVisible())
    {
      delete m_confirmMenu;
      m_confirmMenu = 0;
    }

    refresh();
  }
  else if (isVisible())
  {
    Item& item = item_ref(get_string_after_first_space(getCurrentMenuChoice()));
    if (item.cost > 0)
    {
      m_confirmMenu = new ConfirmMenu(ConfirmMenu::SELL, item.name);
      m_confirmMenu->setVisible(true);
    }
    else
    {
      play_sound(config::get("SOUND_CANCEL"));
    }
  }
}

void ShopSellMenu::handleEscape()
{
  if (m_confirmMenu)
  {
    delete m_confirmMenu;
    m_confirmMenu = 0;
  }
  else
  {
    Menu::handleEscape();
  }
}

void ShopSellMenu::moveArrow(Direction dir)
{
  if (m_confirmMenu)
  {
    m_confirmMenu->moveArrow(dir);
  }
  else
  {
    Menu::moveArrow(dir);
  }
}

void ShopSellMenu::draw(sf::RenderTarget& target, int x, int y)
{
  std::string itemName = get_string_after_first_space(getCurrentMenuChoice());
  Item& item = item_ref(itemName);
  draw_text_bmp(target, 8, 8, "%s", item.description.c_str());

  Menu::draw(target, x, y);

  draw_frame(target, 0, config::GAME_RES_Y - 16, config::GAME_RES_X, 16);
  draw_text_bmp(target, 8, config::GAME_RES_Y - 12, "%s %d", vocab_upcase(terms::gold).c_str(), get_player()->getGold());

  if (m_confirmMenu)
  {
    m_confirmMenu->draw(target,
        config::GAME_RES_X / 2 - m_confirmMenu->getWidth() / 2,
        config::GAME_RES_Y / 2 - m_confirmMenu->getHeight() / 2);
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

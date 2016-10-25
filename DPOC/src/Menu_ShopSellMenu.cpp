#include <functional>

#include "Item.h"
#include "Config.h"
#include "Player.h"
#include "Sound.h"
#include "Utility.h"
#include "Vocabulary.h"
#include "GuiStack.h"

#include "Frame.h"
#include "draw_text.h"

#include "Menu_ShopConfirmMenu.h"
#include "Menu_ShopSellMenu.h"

ShopSellMenu::ShopSellMenu()
  : m_presenter(MenuPresenter::NO_STYLE)
{
  refresh();
}

bool ShopSellMenu::handleInput(sf::Keyboard::Key key)
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

void ShopSellMenu::draw(sf::RenderTarget& target)
{
  const int x = 0;
  const int y = 24;

  draw_frame(target, 0, 0, target.getSize().x, target.getSize().y);

  std::string itemName = get_string_after_first_space(m_presenter.getSelectedOption().entryName);
  Item& item = item_ref(itemName);
  draw_text_bmp(target, 8, 8, "%s", item.description.c_str());

  draw_frame(target, x, y, getWidth(), getHeight());
  m_presenter.draw(target, x, y, this);

  draw_frame(target, 0, config::GAME_RES_Y - 16, config::GAME_RES_X, 16);
  draw_text_bmp(target, 8, config::GAME_RES_Y - 12, "%s %d", vocab_upcase(terms::gold).c_str(), get_player()->getGold());
}

void ShopSellMenu::refresh()
{
  int currentIndex = m_presenter.getSelectedOption().entryIndex;

  m_presenter.clear();

  for (auto it = get_player()->getInventory().begin(); it != get_player()->getInventory().end(); ++it)
  {
    std::string itemName = it->name;

    std::string stack = toString(it->stackSize);
    while (stack.size() < 2)
    {
      stack += ' ';
    }

    std::string entry = stack + " " + itemName;
    m_presenter.addEntry(entry);
  }

  if (get_player()->getInventory().empty())
  {
    close();
  }
  else
  {
    m_presenter.setMaxVisible(7);

    // Fix when selling last entry.
    int currentSize = static_cast<int>(get_player()->getInventory().size());
    if (currentIndex >= currentSize)
    {
      currentIndex = currentSize - 1;
    }

    m_presenter.setSelectedIndex(currentIndex);
  }
}

int ShopSellMenu::getWidth() const
{
  return config::GAME_RES_X;
}

int ShopSellMenu::getHeight() const
{
  return 8 * ENTRY_OFFSET;
}

void ShopSellMenu::handleConfirm()
{
  Item& item = item_ref(get_string_after_first_space(m_presenter.getSelectedOption().entryName));

  if (item.cost > 0)
  {
    getGuiStack()->addWidget<ShopConfirmMenu>(ShopConfirmMenu::SELL, item.name,
      std::bind(&ShopSellMenu::refresh, this));
  }
  else
  {
    play_sound(config::get("SOUND_CANCEL"));
  }
}

void ShopSellMenu::handleEscape()
{
  close();
}

#include "Item.h"
#include "Sound.h"
#include "Config.h"
#include "Player.h"

#include "Frame.h"
#include "draw_text.h"

#include "Menu_ShopConfirmMenu.h"

ShopConfirmMenu::ShopConfirmMenu(BuyOrSell type, const std::string& itemName, const ConfirmCallback confirmCallback)
  : m_quantity(1),
    m_type(type),
    m_itemName(itemName),
    m_presenter(MenuPresenter::STYLE_FRAME),
    m_callback(confirmCallback)
{
  m_presenter.addEntry(type == BUY ? "Buy" : "Sell");
  m_presenter.addEntry("Cancel");
}

bool ShopConfirmMenu::handleInput(sf::Keyboard::Key key)
{
  switch (key)
  {
  case sf::Keyboard::Up:
    m_presenter.scrollUp();
    break;
  case sf::Keyboard::Down:
    m_presenter.scrollDown();
    break;
  case sf::Keyboard::Left:
    m_quantity--;
    if (m_quantity < 1)
    {
      m_quantity = 1;
    }
    break;
  case sf::Keyboard::Right:
    m_quantity++;

    if (m_type == SELL && m_quantity > get_player()->getItem(m_itemName)->stackSize)
    {
      m_quantity = get_player()->getItem(m_itemName)->stackSize;
    }
    else if (m_quantity > 99)
    {
      m_quantity = 99;
    }
    break;
  case sf::Keyboard::Space:
  case sf::Keyboard::Return:
    handleConfirm();
    break;
  case sf::Keyboard::Escape:
    close();
    break;
  default:
    break;
  }

  return true;
}

void ShopConfirmMenu::draw(sf::RenderTarget& target)
{
  int owned = 0;
  if (get_player()->getItem(m_itemName))
  {
    owned = get_player()->getItem(m_itemName)->stackSize;
  }

  const int x = target.getSize().x / 2 - getWidth() / 2;
  const int y = target.getSize().y / 2 - m_presenter.getHeight();

  draw_frame(target, x, y, getWidth(), 40);
  draw_text_bmp(target, x + 8, y + 8,  "Quantity %d", m_quantity);
  draw_text_bmp(target, x + 8, y + 16, "Owned    %d", owned);
  draw_text_bmp(target, x + 8, y + 24, "Sum %d", getSum());

  m_presenter.draw(target, x, y + 40, this);
}

void ShopConfirmMenu::handleConfirm()
{
  std::string choice = m_presenter.getSelectedOption().entryName;

  if (choice == "Buy")
  {
    int stackAfterBuy = m_quantity;
    if (get_player()->getItem(m_itemName))
    {
      stackAfterBuy += get_player()->getItem(m_itemName)->stackSize;
    }

    if (get_player()->getGold() >= getSum() && stackAfterBuy <= 99)
    {
      get_player()->gainGold(-getSum());
      get_player()->addItemToInventory(m_itemName, m_quantity);

      play_sound(config::get("SOUND_SHOP"));

      close();
    }
    else
    {
      play_sound(config::get("SOUND_CANCEL"));
    }
  }
  else if (choice == "Sell")
  {
    get_player()->gainGold(getSum());
    get_player()->removeItemFromInventory(m_itemName, m_quantity);

    play_sound(config::get("SOUND_SHOP"));

    if (m_callback)
    {
      m_callback();
    }

    close();
  }
  else if (choice == "Cancel")
  {
    close();
  }
}

int ShopConfirmMenu::getSum() const
{
  Item& item = item_ref(m_itemName);

  int sum;

  if (m_type == BUY)
  {
    sum = item.cost * m_quantity;
  }
  else
  {
    sum = item.cost * m_quantity / 2;
  }

  return sum;
}

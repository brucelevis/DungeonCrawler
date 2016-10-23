#include "Game.h"
#include "Frame.h"
#include "Player.h"
#include "Utility.h"
#include "draw_text.h"
#include "GuiStack.h"
#include "Menu_ItemMenu.h"

ItemMenu::ItemMenu(const Callback& callback, int x, int y, int w, int h)
  : m_x(x),
    m_y(y),
    m_width(w),
    m_height(h),
    m_callback(callback)
{
  refresh();
}

bool ItemMenu::handleInput(sf::Keyboard::Key key)
{
  switch (key)
  {
  case sf::Keyboard::Up:
    m_itemRange.subIndex(1, Range::WRAP);
    break;
  case sf::Keyboard::Down:
    m_itemRange.addIndex(1, Range::WRAP);
    break;
  case sf::Keyboard::Space:
  case sf::Keyboard::Return:
    if (m_callback)
    {
      m_callback(getSelectedItemName());
    }
    break;
  case sf::Keyboard::Escape:
    getGuiStack()->removeWidget(this);
    break;
  default:
    break;
  }

  return true;
}

void ItemMenu::draw(sf::RenderTarget& target)
{
  const int descriptionPanelHeight = 24;
  draw_frame(target, m_x, m_y, m_width, descriptionPanelHeight);
  draw_frame(target, m_x, m_y + descriptionPanelHeight, m_width, m_height);

  int x = m_x;
  int y = m_y + descriptionPanelHeight;

  for (int index = m_itemRange.getStart(), i = 0; index <= m_itemRange.getEnd(); index++, i++)
  {
    if (index < (int)m_items.size())
    {
      if (const Item* item = getItem(m_items[index]))
      {
        std::string stack = toString(item->stackSize);

        // Add some padding.
        if (stack.size() == 1)
          stack += " ";

        draw_text_bmp(target, x + 16, y + 8 + i * ENTRY_OFFSET, "%s %s", stack.c_str(), item->name.c_str());
      }
      else
      {
        draw_text_bmp(target, x + 16, y + 8 + i * ENTRY_OFFSET, "%s", m_items[index].c_str());
      }
    }

    if (m_itemRange.getIndex() == index && cursorVisible())
    {
      drawSelectArrow(target, x + 8, y + 8 + i * ENTRY_OFFSET);
    }
  }

  if (m_itemRange.getStart() > m_itemRange.getMin())
  {
    drawTopScrollArrow(target, x + m_width - 12, y + 4);
  }

  if (m_itemRange.getEnd() < m_itemRange.getMax())
  {
    drawBottomScrollArrow(target, x + m_width - 12, y + m_height - 12);
  }

  if (cursorVisible() && hasItem(getSelectedItemName()))
  {
    draw_text_bmp(target, m_x + 8, m_y + 8, "%s", getItem(getSelectedItemName())->description.c_str());
  }
}

void ItemMenu::refresh()
{
  m_items.clear();

  const std::vector<Item>& items = Game::instance().getPlayer()->getInventory();

  for (const auto& item : items)
  {
    m_items.push_back(item.name);
  }

  fixRange();
}

void ItemMenu::fixRange()
{
  if (m_itemRange.getIndex() >= static_cast<int>(m_items.size()))
  {
    m_itemRange.reset();
  }

  int visible = m_height / 12 - 1;

  m_itemRange = Range{0, static_cast<int>(m_items.size()), visible};
}

std::string ItemMenu::getSelectedItemName() const
{
  return m_items[m_itemRange.getIndex()];
}

bool ItemMenu::hasItem(const std::string& name) const
{
  for (const auto& s : m_items)
  {
    if (s == name)
      return true;
  }
  return false;
}

const Item* ItemMenu::getItem(const std::string& name) const
{
  for (const auto& item : get_player()->getInventory())
  {
    if (item.name == name)
    {
      return &item;
    }
  }

  return nullptr;
}

#include "Frame.h"
#include "draw_text.h"
#include "Menu_BattleActionMenu.h"

BattleActionMenu::BattleActionMenu(const ConfirmCallback& confirmCallback, const EscapeCallback& escapeCallback, int x, int y)
  : m_confirmCallback(confirmCallback),
    m_escapeCallback(escapeCallback),
    m_x(x),
    m_y(y)
{
  addEntry("Attack");
  addEntry("Spell");
  addEntry("Item");
  addEntry("Guard");
  addEntry("Run");

  m_range = Range{0, m_options.size(), 4};
}

bool BattleActionMenu::handleInput(sf::Keyboard::Key key)
{
  switch (key)
  {
  case sf::Keyboard::Up:
    m_range.subIndex(1, Range::WRAP);
    break;
  case sf::Keyboard::Down:
    m_range.addIndex(1, Range::WRAP);
    break;
  case sf::Keyboard::Space:
  case sf::Keyboard::Return:
    if (m_confirmCallback)
    {
      m_confirmCallback(m_options[m_range.getIndex()]);
    }
    break;
  case sf::Keyboard::Escape:
    if (m_escapeCallback)
    {
      m_escapeCallback();
    }
    break;
  default:
    break;
  }

  return true;
}

void BattleActionMenu::draw(sf::RenderTarget& target)
{
  const int width = 4 + get_longest_string(m_options).size() * 8;
  const int height = 2 * 8 + m_range.getRangeLength() * ENTRY_OFFSET;
  const int x = m_x;
  const int y = m_y;

  draw_frame(target, x, y, width, height);

  for (int index = m_range.getStart(), i = 0; index <= m_range.getEnd(); index++, i++)
  {
    if (index < (int)m_options.size())
    {
      draw_text_bmp(target, x + 16, y + 8 + i * ENTRY_OFFSET, "%s", m_options[index].c_str());
    }

    if (m_range.getIndex() == index && cursorVisible())
    {
      drawSelectArrow(target, x + 8, y + 8 + i * ENTRY_OFFSET);
    }
  }

  if (m_range.getStart() > m_range.getMin())
  {
    drawTopScrollArrow(target, x + width - 12, y + 4);
  }

  if (m_range.getEnd() < m_range.getMax())
  {
    drawBottomScrollArrow(target, x + width - 12, y + height - 12);
  }
}

void BattleActionMenu::addEntry(const std::string& option)
{
  m_options.push_back(option);
}

void BattleActionMenu::init(PlayerCharacter* character)
{
  m_options.clear();

  for (auto it = character->getClass().battleActions.begin();
       it != character->getClass().battleActions.end();
       ++it)
  {
    addEntry(*it);
  }

  m_range = Range{0, m_options.size(), 4};
}

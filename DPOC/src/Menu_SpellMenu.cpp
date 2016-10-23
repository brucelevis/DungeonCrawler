#include "Player.h"
#include "Frame.h"
#include "Utility.h"
#include "draw_text.h"
#include "GuiStack.h"
#include "Menu_SpellMenu.h"

SpellMenu::SpellMenu(const Callback& callback, const std::string& characterName, int x, int y)
  : m_x(x),
    m_y(y),
    m_width(14*16),
    m_height(12*16),
    m_callback(callback)
{
  const std::vector<std::string>& spells = get_player()->getCharacter(characterName)->getSpells();

  for (const std::string& spellName : spells)
  {
    if (const Spell* spell = get_spell(spellName))
    {
      m_spells.push_back(spell);
    }
  }

  const int maxVisible = 10;

  m_range = Range{0, static_cast<int>(m_spells.size()), maxVisible};
}

bool SpellMenu::handleInput(sf::Keyboard::Key key)
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
    if (m_callback && getSelectedSpell())
    {
      m_callback(getSelectedSpell());
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

void SpellMenu::draw(sf::RenderTarget& target)
{
  const int descriptionPanelHeight = 24;
  draw_frame(target, m_x, m_y, m_width, descriptionPanelHeight);
  draw_frame(target, m_x, m_y + descriptionPanelHeight, m_width, m_height);

  int x = m_x;
  int y = m_y + descriptionPanelHeight;

  for (int index = m_range.getStart(), i = 0; index <= m_range.getEnd(); index++, i++)
  {
    if (index < (int)m_spells.size())
    {
      std::string mpCost = toString(m_spells[index]->mpCost);

      // Add some padding.
      if (mpCost.size() == 1)
        mpCost += " ";

      draw_text_bmp(target, x + 16, y + 8 + i * ENTRY_OFFSET, "%s %s", mpCost.c_str(), m_spells[index]->name.c_str());
    }

    if (m_range.getIndex() == index && cursorVisible())
    {
      drawSelectArrow(target, x + 8, y + 8 + i * ENTRY_OFFSET);
    }
  }

  if (m_range.getStart() > m_range.getMin())
  {
    drawTopScrollArrow(target, x + m_width - 12, y + 4);
  }

  if (m_range.getEnd() < m_range.getMax())
  {
    drawBottomScrollArrow(target, x + m_width - 12, y + m_height - 12);
  }

  if (cursorVisible() && getSelectedSpell())
  {
    draw_text_bmp(target, m_x + 8, m_y + 8, "%s", getSelectedSpell()->description.c_str());
  }
}

const Spell* SpellMenu::getSelectedSpell() const
{
  if (m_spells.empty())
  {
    return nullptr;
  }

  return m_spells[m_range.getIndex()];
}

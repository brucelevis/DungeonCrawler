#include "Frame.h"
#include "Utility.h"
#include "GuiWidget.h"
#include "draw_text.h"

#include "MenuPresenter.h"

MenuPresenter::MenuPresenter(Style style)
  : m_style(style)
{
}

void MenuPresenter::clear()
{
  m_range = Range();
  m_options.clear();
}

void MenuPresenter::reset()
{
  m_range.moveTo(0);
}

void MenuPresenter::scrollUp()
{
  m_range.subIndex(1, Range::WRAP);
}

void MenuPresenter::scrollDown()
{
  m_range.addIndex(1, Range::WRAP);
}

void MenuPresenter::setMaxVisible(int maxVisible)
{
  m_range = Range(0, static_cast<int>(m_options.size()), maxVisible);
}

void MenuPresenter::addEntry(const std::string& entryName)
{
  m_options.push_back(entryName);
  m_range = Range(0, static_cast<int>(m_options.size()), static_cast<int>(m_options.size()));
}

int MenuPresenter::getWidth() const
{
  return (4 + get_longest_string(m_options).size()) * 8;
}

int MenuPresenter::getHeight() const
{
  return 2 * 8 + m_range.getRangeLength() * ENTRY_OFFSET;
}

MenuPresenter::Entry MenuPresenter::getSelectedOption() const
{
  Entry entry = { m_options[m_range.getIndex()], m_range.getIndex() };
  return entry;
}

void MenuPresenter::setSelectedIndex(int index)
{
  m_range.moveTo(index);
}

int MenuPresenter::getNumberOfOptions() const
{
  return static_cast<int>(m_options.size());
}

void MenuPresenter::draw(sf::RenderTarget& target, int x, int y, const GuiWidget* guiWidget) const
{
  const int width = getWidth();
  const int height = getHeight();

  if (m_style == STYLE_FRAME)
  {
    draw_frame(target, x, y, width, height);
  }

  for (int index = m_range.getStart(), i = 0; index < m_range.getEnd(); index++, i++)
  {
    if (index < (int)m_options.size())
    {
      draw_text_bmp(target, x + 16, y + 8 + i * ENTRY_OFFSET, "%s", m_options[index].c_str());
    }

    if (m_range.getIndex() == index && guiWidget->cursorVisible())
    {
      guiWidget->drawSelectArrow(target, x + 8, y + 8 + i * ENTRY_OFFSET);
    }
  }

  if (m_range.getStart() > m_range.getMin())
  {
    guiWidget->drawTopScrollArrow(target, x + width - 12, y + 4);
  }

  if (m_range.getEnd() < m_range.getMax())
  {
    guiWidget->drawBottomScrollArrow(target, x + width - 12, y + height - 12);
  }
}

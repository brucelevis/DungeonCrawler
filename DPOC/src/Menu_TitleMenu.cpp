#include "Frame.h"
#include "SaveMenu.h"
#include "draw_text.h"
#include "Menu_TitleMenu.h"

TitleMenu::TitleMenu(const Callback& callback)
  : m_callback(callback)
{
  m_options.push_back("New Game");
  m_options.push_back("Load Game");
  m_options.push_back("Exit");

  m_range = Range{0, m_options.size(), m_options.size()};
}

bool TitleMenu::handleInput(sf::Keyboard::Key key)
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
    handleConfirm(m_options[m_range.getIndex()]);
    break;
  case sf::Keyboard::Escape:
    break;
  default:
    break;
  }

  return true;
}

void TitleMenu::handleConfirm(const std::string& option)
{
  if (option == "New Game")
  {
    m_callback(NEW_GAME);
  }
  else if (option == "Exit")
  {
    m_callback(EXIT_GAME);
  }
  else
  {
    auto loadMenu = getGuiStack()->addWidget<SaveMenu>(SaveMenu::LOAD);
    loadMenu->setLoadGameCallback(std::bind(&TitleMenu::gameLoaded, this));
  }
}

void TitleMenu::draw(sf::RenderTarget& target)
{
  const int width = 4 + get_longest_string(m_options).size() * 8;
  const int height = 2 * 8 + m_range.getRangeLength() * ENTRY_OFFSET;
  const int x = target.getSize().x / 2 - width / 2;
  const int y = target.getSize().y / 2 - height / 2;

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
}

void TitleMenu::gameLoaded()
{
  getGuiStack()->removeWidget(getGuiStack()->findWidget<SaveMenu>());
  m_callback(LOAD_GAME);
}

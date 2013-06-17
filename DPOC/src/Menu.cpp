#include "logger.h"
#include "Cache.h"
#include "Frame.h"
#include "draw_text.h"
#include "Menu.h"

static std::string get_longest_menu_choice(const std::vector<std::string>& menuChoices)
{
  std::string longest;

  for (auto it = menuChoices.begin(); it != menuChoices.end(); ++it)
  {
    if (it->size() > longest.size())
      longest = *it;
  }

  return longest;
}

Menu::Menu()
 : m_arrowTexture(cache::loadTexture("Resources/Arrow.png")),
   m_visible(false),
   m_currentMenuChoice(0),
   m_maxVisible(-1),
   m_scroll(0)
{

}

Menu::~Menu()
{
  cache::releaseTexture("Resources/Arrow.png");
}

void Menu::moveArrow(Direction dir)
{
  if (dir == DIR_UP)
  {
    m_currentMenuChoice--;
    if (m_currentMenuChoice < 0)
    {
      m_currentMenuChoice = 0;
    }

    if (m_maxVisible != -1)
    {
      if (m_currentMenuChoice < m_scroll)
      {
        m_scroll--;
      }
    }
  }
  else if (dir == DIR_DOWN)
  {
    m_currentMenuChoice++;
    if (m_currentMenuChoice >= (int)m_menuChoices.size())
    {
      m_currentMenuChoice = m_menuChoices.size() - 1;
    }

    if (m_maxVisible != -1)
    {
      if (m_currentMenuChoice >= m_maxVisible + m_scroll)
      {
        m_scroll++;
      }
    }
  }
}

void Menu::draw(sf::RenderTarget& target, int x, int y)
{
  int start = m_maxVisible == -1 ? 0 : m_scroll;
  int end = m_maxVisible == -1 ? m_menuChoices.size() : m_maxVisible;

  int w = (4 + get_longest_menu_choice(m_menuChoices).size()) * 8;
  int h = (2 + end) * 8;

  if (m_maxVisible != -1)
    end += m_scroll;

  draw_frame(target, x, y, w, h);

  int i = 0;
  for (int index = start; index < end; index++, i++)
  {
    if (index < (int)m_menuChoices.size())
    {
      draw_text_bmp(target, x + 16, y + 8 + i * 8, "%s", m_menuChoices[index].c_str());
    }

    if (m_currentMenuChoice == index)
    {
      sf::Sprite sprite;
      sprite.setTexture(*m_arrowTexture);
      sprite.setTextureRect(sf::IntRect(0, 0, 8, 8));
      sprite.setPosition(x + 8, y + 8 + i * 8);
      target.draw(sprite);
    }
  }

  if (m_scroll > 0)
  {
    sf::Sprite sprite;
    sprite.setTexture(*m_arrowTexture);
    sprite.setPosition(x + w - 12, y + 4);
    sprite.setTextureRect(sf::IntRect(8, 0, 8, 8));
    target.draw(sprite);
  }

  if (end < (int)m_menuChoices.size())
  {
    sf::Sprite sprite;
    sprite.setTexture(*m_arrowTexture);
    sprite.setPosition(x + w - 12, y + h - 12);
    sprite.setTextureRect(sf::IntRect(16, 0, 8, 8));
    target.draw(sprite);
  }

}

MainMenu::MainMenu()
{
  addEntry("Item");
  addEntry("Spell");
  addEntry("Equip");
  addEntry("Status");
  addEntry("Close");
}

void MainMenu::handleConfirm()
{
  if (currentMenuChoice() == "Close")
  {
    setVisible(false);
  }
}

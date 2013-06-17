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
   m_currentMenuChoice(0)
{

}

Menu::~Menu()
{
  cache::releaseTexture("Resources/Arrow.png");
}

void Menu::handleConfirm()
{
  if (currentMenuChoice() == "Close")
  {
    setVisible(false);
  }
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
  }
  else if (dir == DIR_DOWN)
  {
    m_currentMenuChoice++;
    if (m_currentMenuChoice >= (int)m_menuChoices.size())
    {
      m_currentMenuChoice = m_menuChoices.size() - 1;
    }
  }
}

void Menu::draw(sf::RenderTarget& target, int x, int y)
{
  draw_frame(target, x, y, (3 + get_longest_menu_choice(m_menuChoices).size()) * 8, (2 + m_menuChoices.size()) * 8);

  for (size_t i = 0; i < m_menuChoices.size(); i++)
  {
    draw_text_bmp(target, x + 16, y + 8 + i * 8, "%s", m_menuChoices[i].c_str());

    if (m_currentMenuChoice == (int)i)
    {
      sf::Sprite sprite;
      sprite.setTexture(*m_arrowTexture);
      sprite.setPosition(x + 8, y + 8 + i * 8);
      target.draw(sprite);
    }
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

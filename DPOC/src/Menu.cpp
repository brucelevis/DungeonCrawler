#include "logger.h"
#include "Cache.h"
#include "Frame.h"
#include "draw_text.h"
#include "Utility.h"

#include "Player.h"
#include "Character.h"
#include "Game.h"
#include "Item.h"

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

int Menu::getWidth() const
{
  return (4 + get_longest_menu_choice(m_menuChoices).size()) * 8;
}

int Menu::getHeight() const
{
  int end = m_maxVisible == -1 ? m_menuChoices.size() : m_maxVisible;
  return (2 + end) * 8;
}

void Menu::draw(sf::RenderTarget& target, int x, int y)
{
  int start = m_maxVisible == -1 ? 0 : m_scroll;
  int end = m_maxVisible == -1 ? m_menuChoices.size() : (m_maxVisible + m_scroll);

  int w = getWidth();
  int h = getHeight();

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
    drawSelectArrow(target, x + w - 12, y + 4);
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

void Menu::drawSelectArrow(sf::RenderTarget& target, int x, int y)
{
  sf::Sprite sprite;
  sprite.setTexture(*m_arrowTexture);
  sprite.setTextureRect(sf::IntRect(0, 0, 8, 8));
  sprite.setPosition(x, y);
  target.draw(sprite);
}

///////////////////////////////////////////////////////////////////////////////

MainMenu::MainMenu()
 : m_itemMenu(0)
{
  addEntry("Item");
  addEntry("Spell");
  addEntry("Equip");
  addEntry("Status");
  addEntry("Close");

  m_stateStack.push(STATE_MAIN_MENU);
}

void MainMenu::handleConfirm()
{
  State currentState = m_stateStack.top();

  if (currentState == STATE_MAIN_MENU)
  {
    if (currentMenuChoice() == "Close")
    {
      setVisible(false);
    }
    else if (currentMenuChoice() == "Item")
    {
      openItemMenu();
    }
    else if (currentMenuChoice() == "Spell")
    {
      openCharacterMenu();
    }
    else if (currentMenuChoice() == "Status")
    {
      openCharacterMenu();
    }
  }
  else if (currentState == STATE_CHARACTER_MENU)
  {
    if (currentMenuChoice() == "Spell")
    {
      openSpellMenu(m_characterMenu->currentMenuChoice());
    }
  }
}

void MainMenu::handleEscape()
{
  State currentState = m_stateStack.top();

  if (currentState != STATE_MAIN_MENU)
  {
    if (currentState == STATE_ITEM_MENU)
    {
      closeItemMenu();
    }
    else if (currentState == STATE_SPELL_MENU)
    {
      closeSpellMenu();
    }
    else if (currentState == STATE_CHARACTER_MENU)
    {
      closeCharacterMenu();
    }

    m_stateStack.pop();
  }
  else
  {
    Menu::handleEscape();
  }
}

void MainMenu::moveArrow(Direction dir)
{
  State currentState = m_stateStack.top();

  if (currentState == STATE_ITEM_MENU)
  {
    m_itemMenu->moveArrow(dir);
  }
  else if (currentState == STATE_CHARACTER_MENU)
  {
    m_characterMenu->moveArrow(dir);
  }
  else if (currentState == STATE_SPELL_MENU)
  {
    m_spellMenu->moveArrow(dir);
  }
  else if (currentState == STATE_MAIN_MENU)
  {
    Menu::moveArrow(dir);
  }
}

void MainMenu::openItemMenu()
{
  m_itemMenu = new ItemMenu;
  m_itemMenu->setVisible(true);

  m_stateStack.push(STATE_ITEM_MENU);
}

void MainMenu::closeItemMenu()
{
  delete m_itemMenu;
  m_itemMenu = 0;
}

void MainMenu::openSpellMenu(const std::string& characterName)
{
  m_spellMenu = new SpellMenu(characterName);
  m_spellMenu->setVisible(true);

  m_stateStack.push(STATE_SPELL_MENU);
}

void MainMenu::closeSpellMenu()
{
  delete m_spellMenu;
  m_spellMenu = 0;
}

void MainMenu::openCharacterMenu()
{
  m_characterMenu = new CharacterMenu;
  m_characterMenu->setVisible(true);

  m_stateStack.push(STATE_CHARACTER_MENU);
}

void MainMenu::closeCharacterMenu()
{
  delete m_characterMenu;
  m_characterMenu = 0;
}

void MainMenu::draw(sf::RenderTarget& target, int x, int y)
{
  Menu::draw(target, x, y);

  if (m_characterMenu)
  {
    m_characterMenu->draw(target, x, y + getHeight() + 8);
  }

  if (m_itemMenu)
  {
    m_itemMenu->draw(target, x + getWidth() + 16, y);
  }

  if (m_spellMenu)
  {
    m_spellMenu->draw(target, x + getWidth() + 16, y);
  }
}

///////////////////////////////////////////////////////////////////////////////

ItemMenu::ItemMenu()
{
  refresh();
}

void ItemMenu::handleConfirm()
{

}

void ItemMenu::refresh()
{
  clear();

  const std::vector<Item*>& items = Game::instance().getPlayer()->getInventory();

  for (auto it = items.begin(); it != items.end(); ++it)
  {
    std::string stack = toString((*it)->stack);
    std::string name = (*it)->getName();

    addEntry(stack + " " + name);
  }

  for (int i = 0; i < 20; i++)
  {
    addEntry(toString(i) + " " + std::string("DummyItem"));
  }

  setMaxVisible(10);
}

///////////////////////////////////////////////////////////////////////////////

SpellMenu::SpellMenu(const std::string& characterName)
{
  const std::vector<std::string>& spells = Game::instance().getPlayer()->getCharacter(characterName)->getSpells();

  for (auto it = spells.begin(); it != spells.end(); ++it)
  {
    addEntry("10 " + *it);
  }

  setMaxVisible(10);
}

void SpellMenu::handleConfirm()
{

}

///////////////////////////////////////////////////////////////////////////////

CharacterMenu::CharacterMenu()
{
  const std::vector<Character*>& party = Game::instance().getPlayer()->getParty();

  for (auto it = party.begin(); it != party.end(); ++it)
  {
    addEntry((*it)->getName());
  }
}

void CharacterMenu::handleConfirm()
{

}

int CharacterMenu::getWidth() const
{
  return Menu::getWidth() + 32;
}

int CharacterMenu::getHeight() const
{
  return (4 + 32) * getNumberOfChoice();
}

void CharacterMenu::draw(sf::RenderTarget& target, int x, int y)
{
//  Menu::draw(target, x, y);

  int w = getWidth();
  int h = getHeight();

  draw_frame(target, x, y, w, h);

  for (int i = 0; i < getNumberOfChoice(); i++)
  {
    const sf::Texture* face = Game::instance().getPlayer()->getCharacter(getChoice(i))->getTexture();

    sf::Sprite sprite;
    sprite.setTexture(*face);
    sprite.setPosition(x + 16, y + 8 + i * 32);
    target.draw(sprite);

    draw_text_bmp(target, x + 52, y + 8 + i * 32, "%s", getChoice(i).c_str());
    draw_text_bmp(target, x + 52, y + 8 + i * 32 + 8, "HP%d", 999);
    draw_text_bmp(target, x + 52, y + 8 + i * 32 + 16, "MP%d", 999);

    if (getCurrentChoiceIndex() == i)
    {
      drawSelectArrow(target, x + 8, y + 8 + i * 32 + 12);
    }
  }
}

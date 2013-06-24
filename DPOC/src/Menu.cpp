#include "Config.h"
#include "logger.h"
#include "Cache.h"
#include "Frame.h"
#include "draw_text.h"
#include "Utility.h"

#include "Player.h"
#include "Character.h"
#include "Game.h"
#include "Item.h"
#include "Spell.h"

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
   m_scroll(0),
   m_cursorVisible(true)
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
      drawSelectArrow(target, x + 8, y + 8 + i * 8);
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
 : m_itemMenu(0),
   m_spellMenu(0),
   m_characterMenu(new CharacterMenu)
{
  addEntry("Item");
  addEntry("Spell");
  addEntry("Equip");
  addEntry("Status");
  addEntry("Save");
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
      if (m_characterMenu->getSpellToUse())
      {
        cast_spell(m_characterMenu->getSpellToUse(),
            Game::instance().getPlayer()->getCharacter(m_characterMenu->currentMenuChoice()),
            Game::instance().getPlayer()->getCharacter(m_characterMenu->currentMenuChoice()));
        closeCharacterMenu();
        m_stateStack.pop();
      }
      else
      {
        m_stateStack.pop();
        openSpellMenu(m_characterMenu->currentMenuChoice());
        closeCharacterMenu();
      }
    }
    else if (currentMenuChoice() == "Status")
    {
      m_stateStack.push(STATE_STATUS_MENU);
    }
  }
  else if (currentState == STATE_SPELL_MENU)
  {
    const Spell* spell = m_spellMenu->getSelectedSpell();
    if (!spell->battleOnly)
    {
      openCharacterMenu();
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
  m_characterMenu->resetChoice();
  m_characterMenu->setCursorVisible(true);
  m_stateStack.push(STATE_CHARACTER_MENU);
}

void MainMenu::closeCharacterMenu()
{
  m_characterMenu->setCursorVisible(false);
}

void MainMenu::open()
{
  setVisible(true);
  resetChoice();

  m_characterMenu->refresh();
  m_characterMenu->setVisible(true);
}

void MainMenu::draw(sf::RenderTarget& target, int x, int y)
{
//  Menu::draw(target, x, y);
  State currentState = m_stateStack.top();

  if (currentState == STATE_ITEM_MENU)
  {

  }
  else if (currentState == STATE_SPELL_MENU)
  {

  }
  else if (currentState == STATE_EQUIP_MENU)
  {

  }
  else
  {
    draw_frame(target, x, y, config::GAME_RES_X, config::GAME_RES_Y);

    draw_frame(target, x, y, 80, 96);
    draw_frame(target, x, y + 208, 80, 32);

    draw_text_bmp(target, x + 8, y + 13*16+7, "GP");
    draw_text_bmp(target, x + 8, y + 13*16+19, "%d", 0);

    for (int i = 0; i < getNumberOfChoice(); i++)
    {
      draw_text_bmp(target, x + 18, y + 10 + i * 14, "%s", getChoice(i).c_str());
      if (i == getCurrentChoiceIndex())
      {
        drawSelectArrow(target, x + 8, y + 10 + i * 14);
      }
    }

    if (m_characterMenu && m_characterMenu->isVisible())
    {
      m_characterMenu->draw(target, x, y);
    }

    if (currentState == STATE_STATUS_MENU)
    {
      int offX = x + 24;
      int offY = y + 24;
    }
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

  const std::vector<Item>& items = Game::instance().getPlayer()->getInventory();

  for (auto it = items.begin(); it != items.end(); ++it)
  {
    std::string stack = toString(it->stackSize);
    std::string name = it->name;

    addEntry(stack + " " + name);
  }

  setMaxVisible(10);
}

///////////////////////////////////////////////////////////////////////////////

SpellMenu::SpellMenu(const std::string& characterName)
{
  const std::vector<std::string>& spells = Game::instance().getPlayer()->getCharacter(characterName)->getSpells();

  for (auto it = spells.begin(); it != spells.end(); ++it)
  {
    const Spell* spell = get_spell(*it);

    if (spell)
    {
      addEntry(toString(spell->mpCost) + " " + spell->name);
    }
  }

  setMaxVisible(10);
}

void SpellMenu::handleConfirm()
{

}

const Spell* SpellMenu::getSelectedSpell() const
{
  std::istringstream ss(currentMenuChoice());

  std::string spellName;
  int mpCost;

  ss >> mpCost >> spellName;

  return get_spell(spellName);
}

///////////////////////////////////////////////////////////////////////////////

CharacterMenu::CharacterMenu()
 : m_spellToUse(0)
{
  m_cursorVisible = false;
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

void CharacterMenu::refresh()
{
  clear();

  const std::vector<Character*>& party = Game::instance().getPlayer()->getParty();

  for (auto it = party.begin(); it != party.end(); ++it)
  {
    addEntry((*it)->getName());
  }
}

void CharacterMenu::draw(sf::RenderTarget& target, int x, int y)
{
  draw_frame(target, x + 72, y, 184, 240);

  for (int i = 0; i < getNumberOfChoice(); i++)
  {
    const Character* character = Game::instance().getPlayer()->getCharacter(getChoice(i));
    const sf::Texture* face = character->getTexture();

    int offX = x + 8 + 5 * 16;
    int offY = y + 8;

    sf::Sprite sprite;
    sprite.setTexture(*face);
    sprite.setPosition(offX, offY + i * 48);
    target.draw(sprite);

    draw_text_bmp(target, offX + 40, offY + i * 48, "%s (%s)", character->getName().c_str(), "Normal");
    draw_text_bmp(target, offX + 40, offY + i * 48 + 12, "Hp: %d/%d", 999, 999);
    draw_text_bmp(target, offX + 40, offY + i * 48 + 24, "Mp: %d/%d", 999, 999);

    if (cursorVisible() && getCurrentChoiceIndex() == i)
    {
      sf::RectangleShape rect;
      rect.setFillColor(sf::Color::Transparent);
      rect.setOutlineThickness(1.0f);
      rect.setOutlineColor(sf::Color::Red);
      rect.setSize(sf::Vector2f(164, 36));
      rect.setPosition(offX - 2, offY + i * 48 - 2);
      target.draw(rect);
    }
  }
}

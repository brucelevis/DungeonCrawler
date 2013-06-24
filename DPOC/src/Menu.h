#ifndef MENU_H
#define MENU_H

#include <string>
#include <vector>
#include <stack>

#include <SFML/Graphics.hpp>

#include "Direction.h"

class Spell;

class Menu
{
public:
  Menu();
  virtual ~Menu();

  void setVisible(bool visible) { m_visible = visible; }
  bool isVisible() const { return m_visible; }

  virtual void handleConfirm() = 0;
  virtual void handleEscape() { setVisible(false); }

  virtual void moveArrow(Direction dir);

  virtual void draw(sf::RenderTarget& target, int x, int y);

  std::string currentMenuChoice() const { return m_menuChoices[m_currentMenuChoice]; }
  std::string getChoice(size_t index) const { return m_menuChoices[index]; }

  void addEntry(const std::string& choice) { m_menuChoices.push_back(choice); }

  void clear() { m_menuChoices.clear(); }

  virtual int getWidth() const;
  virtual int getHeight() const;

  int getNumberOfChoice() const { return m_menuChoices.size(); }

  void setCursorVisible(bool visible) { m_cursorVisible = visible; }
  bool cursorVisible() const { return m_cursorVisible; }

  void resetChoice() { m_currentMenuChoice = 0; }
protected:
  void setMaxVisible(int maxVisible) { m_maxVisible = maxVisible; }
  int getCurrentChoiceIndex() const { return m_currentMenuChoice; }
  void drawSelectArrow(sf::RenderTarget& target, int x, int y);
private:
  sf::Texture* m_arrowTexture;
  bool m_visible;
  int m_currentMenuChoice;
  std::vector<std::string> m_menuChoices;

  int m_maxVisible;
  int m_scroll;

  bool m_cursorVisible;
};

class MainMenu;
class ItemMenu;
class SpellMenu;
class CharacterMenu;

class MainMenu : public Menu
{
  enum State
  {
    STATE_MAIN_MENU,
    STATE_ITEM_MENU,
    STATE_CHARACTER_MENU,
    STATE_EQUIP_MENU,
    STATE_SPELL_MENU,
    STATE_STATUS_MENU,
    STATE_SAVE_MENU
  };
public:
  MainMenu();

  void handleConfirm();
  void handleEscape();

  void moveArrow(Direction dir);

  void open();

  void draw(sf::RenderTarget& target, int x, int y);
private:
  void openItemMenu();
  void closeItemMenu();

  void openSpellMenu(const std::string& characterName);
  void closeSpellMenu();

  void openCharacterMenu();
  void closeCharacterMenu();
private:
  ItemMenu* m_itemMenu;
  SpellMenu* m_spellMenu;
  CharacterMenu* m_characterMenu;

  std::stack<State> m_stateStack;
};

class ItemMenu : public Menu
{
public:
  ItemMenu();

  void handleConfirm();

  void refresh();
private:
};

class SpellMenu : public Menu
{
public:
  SpellMenu(const std::string& characterName);

  void handleConfirm();

  const Spell* getSelectedSpell() const;
private:
};

class CharacterMenu : public Menu
{
public:
  CharacterMenu();

  void handleConfirm();

  int getWidth() const;
  int getHeight() const;

  void refresh();

  void draw(sf::RenderTarget& target, int x, int y);

  void setSpellToUse(const Spell* spell) { m_spellToUse = spell; }
  const Spell* getSpellToUse() const { return m_spellToUse; }
private:
  const Spell* m_spellToUse;
};

#endif

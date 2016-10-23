#ifndef MENU_H
#define MENU_H

#include <string>
#include <vector>
#include <stack>
#include <map>

#include <SFML/Graphics.hpp>

#include "Target.h"
#include "Direction.h"

class Item;
class Spell;
class Character;
class PlayerCharacter;
class Battle;
class SaveMenu;

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

  std::string getCurrentMenuChoice() const
  {
    if (m_menuChoices.empty())
      return "";

    return m_menuChoices[m_currentMenuChoice];
  }
  std::string getChoice(size_t index) const { return m_menuChoices[index]; }

  void addEntry(const std::string& choice) { m_menuChoices.push_back(choice); }

  void clear() { m_menuChoices.clear(); }

  virtual int getWidth() const;
  virtual int getHeight() const;

  int getNumberOfChoice() const { return m_menuChoices.size(); }

  void setCursorVisible(bool visible) { m_cursorVisible = visible; }
  bool cursorVisible() const { return m_cursorVisible; }

  virtual void resetChoice() { m_currentMenuChoice = 0; resetScroll(); }

  int getCurrentChoiceIndex() const { return m_currentMenuChoice; }
  void setCurrentChoice(int choice) { m_currentMenuChoice = choice; }
protected:
  void setMaxVisible(int maxVisible) { m_maxVisible = maxVisible; }
  void drawSelectArrow(sf::RenderTarget& target, int x, int y);
private:
  void fixScroll(Direction dir);
  void resetScroll() {m_scroll = 0; }
private:
  sf::Texture* m_arrowTexture;
  bool m_visible;
  int m_currentMenuChoice;
  std::vector<std::string> m_menuChoices;

  int m_maxVisible;
  int m_scroll;

  bool m_cursorVisible;
};

class ChoiceMenu : public Menu
{
public:
  void handleConfirm();
};

#endif

#ifndef MENU_H
#define MENU_H

#include <string>
#include <vector>
#include <stack>

#include <SFML/Graphics.hpp>

#include "Direction.h"

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

  void addEntry(const std::string& choice) { m_menuChoices.push_back(choice); }

  void clear() { m_menuChoices.clear(); }

  int getWidth() const;
  int getHeight() const;
protected:
  void setMaxVisible(int maxVisible) { m_maxVisible = maxVisible; }
private:
  sf::Texture* m_arrowTexture;
  bool m_visible;
  int m_currentMenuChoice;
  std::vector<std::string> m_menuChoices;

  int m_maxVisible;
  int m_scroll;
};

class MainMenu;
class ItemMenu;

class MainMenu : public Menu
{
  enum State
  {
    STATE_MAIN_MENU,
    STATE_ITEM_MENU,
    STATE_CHARACTER_MENU,
    STATE_EQUIP_MENU,
    STATE_SPELL_MENU,
    STATE_STATUS_MENU
  };
public:
  MainMenu();

  void handleConfirm();
  void handleEscape();

  void moveArrow(Direction dir);

  void draw(sf::RenderTarget& target, int x, int y);
private:
  void openItemMenu();
  void closeItemMenu();
private:
  ItemMenu* m_itemMenu;

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

#endif

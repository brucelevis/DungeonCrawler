#ifndef MENU_H
#define MENU_H

#include <string>
#include <vector>

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

  void moveArrow(Direction dir);

  void draw(sf::RenderTarget& target, int x, int y);

  std::string currentMenuChoice() const { return m_menuChoices[m_currentMenuChoice]; }

  void addEntry(const std::string& choice) { m_menuChoices.push_back(choice); }
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

class MainMenu : public Menu
{
public:
  MainMenu();

  virtual void handleConfirm();
private:
};

#endif

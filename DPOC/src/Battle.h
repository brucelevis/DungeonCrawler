#ifndef BATTLE_H
#define BATTLE_H

#include <SFML/Graphics.hpp>

#include "Menu.h"

class Battle
{
public:
  Battle(sf::RenderWindow& window);

  void start();
private:
  void pollEvents();

  void handleKeyPress(sf::Keyboard::Key key);

  void draw();
private:
  bool m_battleOngoing;

  BattleMenu m_battleMenu;

  sf::RenderWindow& m_window;
};

#endif

#ifndef BATTLE_H
#define BATTLE_H

#include <vector>

#include <SFML/Graphics.hpp>

#include "Menu.h"

class Character;

class Battle
{
public:
  Battle(sf::RenderWindow& window, const std::vector<Character*>& monsters);

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

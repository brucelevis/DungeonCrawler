#ifndef GAME_H
#define GAME_H

#include <SFML/Graphics.hpp>
#include "coord.h"

class Map;
class Player;

class Game
{
public:
  static Game& instance();

  void run();

  Map* getCurrentMap() { return m_currentMap; }
  Player* getPlayer() { return m_player; }
private:
  Game();
  ~Game();

  void pollEvents();
  void draw();
private:
  sf::RenderWindow m_window;
  Map* m_currentMap;
  Player* m_player;
  coord_t m_view;
};

#endif

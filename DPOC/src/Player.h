#ifndef PLAYER_H
#define PLAYER_H

class Entity;

#include <vector>
#include <SFML/Graphics.hpp>

#include "coord.h"

class Player
{
public:
  ~Player();

  Entity* player();

  void update();
  void draw(sf::RenderTarget& target, const coord_t& view);

  static Player* create(int x, int y);
private:
  std::vector<Entity*> m_playerTrain;
};

#endif

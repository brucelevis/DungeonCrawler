#include <SFML/Window.hpp>

#include "Direction.h"
#include "Entity.h"
#include "Player.h"

Player::~Player()
{
  for (auto it = m_playerTrain.begin(); it != m_playerTrain.end(); ++it)
  {
    delete *it;
  }
}

Entity* Player::player()
{
  return m_playerTrain.front();
}

void Player::update()
{
  if (sf::Keyboard::isKeyPressed(sf::Keyboard::Right))
  {
    player()->step(DIR_RIGHT);
  }
  else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Left))
  {
    player()->step(DIR_LEFT);
  }
  else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Down))
  {
    player()->step(DIR_DOWN);
  }
  else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Up))
  {
    player()->step(DIR_UP);
  }

  for (auto it = m_playerTrain.begin(); it != m_playerTrain.end(); ++it)
  {
    (*it)->update();
  }
}

void Player::draw(sf::RenderTarget& target, const coord_t& view)
{
  for (auto it = m_playerTrain.begin(); it != m_playerTrain.end(); ++it)
  {
    (*it)->draw(target, view);
  }
}

Player* Player::create(int x, int y)
{
  Player* player = new Player;
  player->m_playerTrain.push_back(new Entity("hero"));
  player->player()->x = x;
  player->player()->y = y;
  return player;
}

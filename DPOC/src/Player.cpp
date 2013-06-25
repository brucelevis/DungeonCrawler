#include <SFML/Window.hpp>

#include <algorithm>

#include "Direction.h"
#include "Entity.h"
#include "Game.h"
#include "Player.h"

Player::~Player()
{
  for (auto it = m_playerTrain.begin(); it != m_playerTrain.end(); ++it)
  {
    delete *it;
  }

  for (auto it = m_party.begin(); it != m_party.end(); ++it)
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

Character* Player::getCharacter(const std::string& name)
{
  for (auto it = m_party.begin(); it != m_party.end(); ++it)
  {
    if ((*it)->getName() == name)
      return *it;
  }

  return 0;
}

void Player::removeItemFromInventory(const std::string& itemName, int number)
{
  for (auto it = m_inventory.begin(); it != m_inventory.end(); ++it)
  {
    if (it->name == itemName)
    {
      it->stackSize -= number;
      if (it->stackSize <= 0)
      {
        m_inventory.erase(it);
      }
      break;
    }
  }
}

Item* Player::getItem(const std::string& itemName)
{
  for (auto it = m_inventory.begin(); it != m_inventory.end(); ++it)
  {
    if (it->name == itemName)
    {
      return &(*it);
    }
  }

  return 0;
}

Player* Player::create(int x, int y)
{
  Player* player = new Player;
  player->m_playerTrain.push_back(new Entity("player"));
  player->player()->setPosition(x, y);

  player->m_party.push_back(Character::create("Char1"));
  player->m_party.push_back(Character::create("Char2"));
  player->m_party.push_back(Character::create("Char3"));
  player->m_party.push_back(Character::create("Char4"));

  player->m_inventory.push_back(create_item("Herb", 3));
  player->m_inventory.push_back(create_item("Rusty Knife", 3));
  player->m_inventory.push_back(create_item("Wood Shield", 21));

  return player;
}

Player* get_player()
{
  return Game::instance().getPlayer();
}

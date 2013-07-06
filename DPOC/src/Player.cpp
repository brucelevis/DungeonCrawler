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
  for (auto it = m_playerTrain.begin(); it != m_playerTrain.end(); ++it)
  {
    if (!(*it)->isWalking())
    {
      m_trainCoords[*it].x = (*it)->x;
      m_trainCoords[*it].y = (*it)->y;
    }
  }

  if (sf::Keyboard::isKeyPressed(sf::Keyboard::Right))
  {
    player()->step(DIR_RIGHT);
    if (player()->isWalking())
      moveTrain();
  }
  else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Left))
  {
    player()->step(DIR_LEFT);
    if (player()->isWalking())
      moveTrain();
  }
  else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Down))
  {
    player()->step(DIR_DOWN);
    if (player()->isWalking())
      moveTrain();
  }
  else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Up))
  {
    player()->step(DIR_UP);
    if (player()->isWalking())
      moveTrain();
  }

  for (auto it = m_playerTrain.begin(); it != m_playerTrain.end(); ++it)
  {
    (*it)->update();
  }
}

void Player::moveTrain()
{
  for (size_t i = 1; i < m_playerTrain.size(); i++)
  {
    Entity* prev = m_playerTrain[i - 1];
    Entity* curr = m_playerTrain[i];

    if ((int)prev->x != (int)curr->x || (int)prev->y != (int)curr->y)
    {
      if (m_trainCoords.count(prev) > 0)
      {
        coord_t coordToFollow = m_trainCoords[prev];

        if (coordToFollow.x < (int)curr->x) curr->step(DIR_LEFT);
        else if (coordToFollow.x > (int)curr->x) curr->step(DIR_RIGHT);
        else if (coordToFollow.y < (int)curr->y) curr->step(DIR_UP);
        else if (coordToFollow.y > (int)curr->y) curr->step(DIR_DOWN);
      }
    }
  }
}

void Player::draw(sf::RenderTarget& target, const coord_t& view)
{
  for (auto it = m_playerTrain.begin(); it != m_playerTrain.end(); ++it)
  {
    (*it)->draw(target, view);
  }
}

PlayerCharacter* Player::getCharacter(const std::string& name)
{
  for (auto it = m_party.begin(); it != m_party.end(); ++it)
  {
    if ((*it)->getName() == name)
      return *it;
  }

  return 0;
}

void Player::addItemToInventory(const std::string& itemName, int number)
{
  if (Item* item = getItem(itemName))
  {
    item->stackSize += number;
  }
  else
  {
    m_inventory.push_back(create_item(itemName, number));
  }
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

void Player::transfer(int x, int y)
{
  for (auto it = m_playerTrain.begin(); it != m_playerTrain.end(); ++it)
  {
    (*it)->setPosition(x, y);
  }
}

void Player::gainExperience(int sum)
{
  for (auto it = m_party.begin(); it != m_party.end(); ++it)
  {
    (*it)->getAttribute("exp").max += sum;
    reset_attribute((*it)->getAttribute("exp"));
  }
}

Player* Player::create(int x, int y)
{
  Player* player = new Player;

  for (size_t i = 0; i < 4; i++)
  {
    player->m_playerTrain.push_back(new Entity("player"));
    player->m_playerTrain[i]->setPosition(x, y);

    if (i > 0)
      player->m_playerTrain[i]->setWalkThrough(true);
  }

  player->m_party.push_back(PlayerCharacter::create("Char1"));
  player->m_party.push_back(PlayerCharacter::create("Char2"));
  player->m_party.push_back(PlayerCharacter::create("Char3"));
  player->m_party.push_back(PlayerCharacter::create("Char4"));

  player->m_inventory.push_back(create_item("Herb", 3));
  player->m_inventory.push_back(create_item("Rusty Knife", 3));
  player->m_inventory.push_back(create_item("Wood Shield", 21));
  player->m_inventory.push_back(create_item("Firebomb", 99));
  player->m_inventory.push_back(create_item("Antidote", 10));
  player->m_inventory.push_back(create_item("Ether", 10));
  player->m_inventory.push_back(create_item("Steroids", 10));

  player->m_gold = 0;

  return player;
}

Player* get_player()
{
  return Game::instance().getPlayer();
}

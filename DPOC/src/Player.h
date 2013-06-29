#ifndef PLAYER_H
#define PLAYER_H

class Entity;

#include <map>
#include <vector>
#include <SFML/Graphics.hpp>

#include "coord.h"
#include "Item.h"
#include "Character.h"

class Player
{
public:
  ~Player();

  Entity* player();

  void update();
  void draw(sf::RenderTarget& target, const coord_t& view);

  const std::vector<Item>& getInventory() const { return m_inventory; }
  const std::vector<Character*>& getParty() const { return m_party; }

  void addItemToInventory(const std::string& itemName, int number);
  void removeItemFromInventory(const std::string& itemName, int number);
  Item* getItem(const std::string& itemName);

  Character* getCharacter(const std::string& name);

  void transfer(int x, int y);

  static Player* create(int x, int y);
private:
  void moveTrain();
private:
  std::vector<Entity*> m_playerTrain;
  std::vector<Item> m_inventory;
  std::vector<Character*> m_party;

  std::map<Entity*, coord_t> m_trainCoords;
};

Player* get_player();

#endif

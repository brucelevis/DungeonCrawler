#ifndef PLAYER_H
#define PLAYER_H

class Entity;
struct CharacterData;
struct EntityData;

#include <map>
#include <vector>
#include <SFML/Graphics.hpp>

#include <BGL/coord.h>

#include "Item.h"
#include "Character.h"
#include "PlayerCharacter.h"

class Player
{
public:
  ~Player();

  Entity* player();

  void update();
  void draw(sf::RenderTarget& target, const bgl::coord_t& view);

  const std::vector<Item>& getInventory() const { return m_inventory; }
  const std::vector<PlayerCharacter*>& getParty() const { return m_party; }
  const std::vector<Entity*>& getTrain() const { return m_playerTrain; }

  void addItemToInventory(const std::string& itemName, int number);
  void removeItemFromInventory(const std::string& itemName, int number);
  Item* getItem(const std::string& itemName);

  PlayerCharacter* getCharacter(const std::string& name);

  void transfer(int x, int y);

  int getGold() const { return m_gold; }
  void gainGold(int sum) { m_gold += sum; }
  void removeGold(int sum)
  {
    m_gold -= sum;
    if (m_gold < 0) m_gold = 0;
  }

  void gainExperience(int sum);

  std::string xmlDump() const;

  void addNewCharacter(const std::string& name, const std::string& className, int x, int y, int level = 1);
  void removeCharacter(const std::string& name);

  void setControlsEnabled(bool enabled) { m_controlsEnabled = enabled; }
  bool isControlsEnabled() const { return m_controlsEnabled; }

  void recoverAll();

  static Player* create();
  static Player* createBlank();
  static Player* createFromSaveData(std::vector<CharacterData*> charData, std::vector<EntityData*> entData);
private:
  Player();

  void moveTrain();
private:
  std::vector<Entity*> m_playerTrain;
  std::vector<Item> m_inventory;
  std::vector<PlayerCharacter*> m_party;

  std::map<Entity*, bgl::coord_t> m_trainCoords;

  int m_gold;

  bool m_controlsEnabled;
};

Player* get_player();

#endif

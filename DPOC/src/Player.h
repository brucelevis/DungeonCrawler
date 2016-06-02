#ifndef PLAYER_H
#define PLAYER_H

class Entity;
struct CharacterData;
struct EntityData;

#include <map>
#include <vector>

#include <SFML/Graphics.hpp>
#include <GameLib/Camera.h>
#include <GameLib/Interpolator.h>

#include "coord.h"
#include "Item.h"
#include "Character.h"
#include "PlayerCharacter.h"
#include "Direction.h"

class Player
{
public:
  ~Player();

  Entity* player();

  void update();
  void draw(sf::RenderTarget& target, const coord_t& view);

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
  void addNewCharacter(const std::string& name, const std::string& className, const std::string& face, int x, int y, int level = 1);
  void removeCharacter(const std::string& name);

  void setControlsEnabled(bool enabled) { m_controlsEnabled = enabled; }
  bool isControlsEnabled() const { return m_controlsEnabled; }

  void recoverAll();

  static Player* create();
  static Player* createBlank();
  static Player* createFromSaveData(std::vector<CharacterData*> charData, std::vector<EntityData*> entData);

  const gamelib::Camera& getCamera() const { return m_camera; }
  void rotate(float angle, float speed);
  bool isRotating() const { return m_isRotating; }
  void initCamera(Direction initDir);
private:
  Player();

  void moveTrain();
  void handleStep();
private:
  gamelib::Camera m_camera;
  gamelib::Interpolator<float> m_rotationInterpolator;

  std::vector<Entity*> m_playerTrain;
  std::vector<Item> m_inventory;
  std::vector<PlayerCharacter*> m_party;

  std::map<Entity*, coord_t> m_trainCoords;

  int m_gold;

  bool m_controlsEnabled;
  bool m_movedBackwards;

  bool  m_isRotating  = false;
  float m_targetAngle = 0;
};

Player* get_player();

#endif

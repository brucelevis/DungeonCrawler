#include <SFML/Window.hpp>

#include <algorithm>
#include <sstream>

#include "SceneManager.h"

#include "Config.h"
#include "SaveLoad.h"
#include "Utility.h"

#include "StatusEffect.h"
#include "Flash.h"

#include "Vocabulary.h"
#include "Sound.h"
#include "Direction.h"
#include "Entity.h"
#include "Sprite.h"
#include "PlayerClass.h"
#include "Game.h"
#include "Player.h"

#include "../dep/tinyxml2.h"

using namespace tinyxml2;

Player::Player()
 : m_camera(glm::vec3(0, 0, 0), glm::pi<float>()),
   m_gold(0),
   m_controlsEnabled(true),
   m_movedBackwards(false)
{
}

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
  if (m_isRotating)
  {
    m_rotationInterpolator.update();
    m_camera.horizontal_angle = m_rotationInterpolator.getCurrent();
    if (m_rotationInterpolator.isDone())
    {
      m_camera.horizontal_angle = m_targetAngle;
      m_isRotating = false;
    }
  }
  else if (m_controlsEnabled)
  {
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Down))
    {
      Direction dir = player()->getDirection();
      Direction tmpDir = dir;
      if (dir == DIR_DOWN) dir = DIR_UP;
      else if (dir == DIR_UP) dir = DIR_DOWN;
      else if (dir == DIR_LEFT) dir = DIR_RIGHT;
      else if (dir == DIR_RIGHT) dir = DIR_LEFT;

      player()->step(dir);

      // Not walking if collision occured.
      if (player()->isWalking())
      {
        m_movedBackwards = true;
      }
      else
      {
        player()->setDirection(tmpDir);
      }
    }
    else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Up))
    {
      player()->step(player()->getDirection());

      m_movedBackwards = false;
    }
  }

  bool wasWalking = player()->isWalking();

  player()->update();

  if (wasWalking && !player()->isWalking() && m_movedBackwards)
  {
    Direction dir = player()->getDirection();
    if (dir == DIR_DOWN) dir = DIR_UP;
    else if (dir == DIR_UP) dir = DIR_DOWN;
    else if (dir == DIR_LEFT) dir = DIR_RIGHT;
    else if (dir == DIR_RIGHT) dir = DIR_LEFT;

    player()->setDirection(dir);
  }

  if (wasWalking && !player()->isWalking())
  {
    handleStep();
  }

  for (PlayerCharacter* pc : m_party)
  {
    if (pc->flash().isShaking())
    {
      pc->flash().update();
    }
  }

  m_camera.position.x = m_playerTrain.front()->x;
  m_camera.position.z = m_playerTrain.front()->y;
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
    if ((*it)->getStatus() != "Dead")
    {
      (*it)->getAttribute("exp").max += sum;
      reset_attribute((*it)->getAttribute("exp"));
    }
  }
}

std::string Player::xmlDump() const
{
  std::ostringstream xml;
  xml << "<player>\n";

  xml << " <party>\n";
  for (size_t i = 0; i < m_party.size(); i++)
  {
    xml << "  <member id=\"" << i << "\">\n";
    xml << m_playerTrain[i]->xmlDump();
    xml << m_party[i]->xmlDump();
    xml << "  </member>\n";
  }
  xml << " </party>\n";

  xml << " <inventory>\n";
  for (auto it = m_inventory.begin(); it != m_inventory.end(); ++it)
  {
    xml << "   <item name=\"" << it->name << "\" stackSize=\"" << it->stackSize << "\" />\n";
  }
  xml << " </inventory>\n";

  xml << "  <gold>" << getGold() << "</gold>\n";

  xml << "</player>\n";
  return xml.str();
}

Player* Player::create()
{
  Player* player = new Player;

  int startX = 0;
  int startY = 0;

  XMLDocument doc;
  doc.LoadFile(config::res_path("Player.xml").c_str());

  const XMLElement* root = doc.FirstChildElement("player");
  const XMLElement* start = root->FirstChildElement("start");
  if (start)
  {
    startX = fromString<int>(start->FindAttribute("x")->Value());
    startY = fromString<int>(start->FindAttribute("y")->Value());
    std::string startMap = start->FindAttribute("map")->Value();

    Game::instance().loadNewMap("Maps/" + startMap);
  }

  const XMLElement* party = root->FirstChildElement("party");
  if (party)
  {
    for (const XMLElement* element = party->FirstChildElement(); element; element = element->NextSiblingElement())
    {
      std::string name = element->FindAttribute("name")->Value();
      std::string className = element->FindAttribute("class")->Value();

      player->addNewCharacter(name, className, startX, startY);
    }
  }

  const XMLElement* inventory = root->FirstChildElement("inventory");
  if (inventory)
  {
    for (const XMLElement* element = inventory->FirstChildElement(); element; element = element->NextSiblingElement())
    {
      std::string name = element->FindAttribute("name")->Value();
      int amount = fromString<int>(element->FindAttribute("amount")->Value());

      player->m_inventory.push_back(create_item(name, amount));
    }
  }

  player->m_gold = 0;

  return player;
}

Player* Player::createBlank()
{
  return new Player;
}

Player* Player::createFromSaveData(std::vector<CharacterData*> charData, std::vector<EntityData*> entData)
{
  Player* player = new Player;

  for (size_t i = 0; i < entData.size(); i++)
  {
    std::string className = charData[i]->className;

    PlayerClass pc = player_class_ref(className);

    Entity* entity = new Entity;

    if (entData[i]->spriteName.size())
    {
      Sprite* sprite = new Sprite;
      sprite->create(pc.texture,
          pc.textureBlock.left, pc.textureBlock.top,
          pc.textureBlock.width, pc.textureBlock.height);

      entity->setSprite(sprite);
    }

    entity->setDirection(entData[i]->dir);
    entity->setWalkSpeed(entData[i]->speed);
    entity->setWalkThrough(entData[i]->walkThrough);
    entity->setPosition(entData[i]->x, entData[i]->y);

    player->m_playerTrain.push_back(entity);
  }

  for (size_t i = 0; i < charData.size(); i++)
  {
    player->m_party.push_back(PlayerCharacter::createFromSaveData(charData[i]));
  }

  return player;
}

Player* get_player()
{
  return Game::instance().getPlayer();
}

void Player::addNewCharacter(const std::string& name, const std::string& className, int x, int y, int level)
{
  PlayerClass pc = player_class_ref(className);

  Entity* entity = new Entity;
  entity->setPosition(x, y);
  entity->setWalkSpeed(0.1f);

  Sprite* sprite = new Sprite;
  sprite->create(pc.texture,
      pc.textureBlock.left, pc.textureBlock.top,
      pc.textureBlock.width, pc.textureBlock.height);

  entity->setSprite(sprite);

  if (m_playerTrain.size() > 0)
    entity->setWalkThrough(true);

  m_playerTrain.push_back(entity);

  m_party.push_back(PlayerCharacter::create(name, className, level));
}

void Player::addNewCharacter(const std::string& name, const std::string& className, const std::string& face, int x, int y, int level)
{
  PlayerClass pc = player_class_ref(className);

  Entity* entity = new Entity;
  entity->setPosition(x, y);
  entity->setWalkSpeed(0.1f);

  m_playerTrain.push_back(entity);

  m_party.push_back(PlayerCharacter::create(name, className, face, level));
}

void Player::removeCharacter(const std::string& name)
{
  int index = 0;
  Entity* entityToRemove = 0;
  for (auto it = m_party.begin(); it != m_party.end(); ++it, ++index)
  {
    if ((*it)->getName() == name)
    {
      m_party.erase(it);
      entityToRemove = m_playerTrain[index];
      break;
    }
  }

  for (auto it = m_playerTrain.begin(); it != m_playerTrain.end(); ++it)
  {
    if ((*it) == entityToRemove)
    {
      m_playerTrain.erase(it);
      break;
    }
  }
}

void Player::recoverAll()
{
  for (auto it = m_party.begin(); it != m_party.end(); ++it)
  {
    reset_attribute((*it)->getAttribute(terms::hp));
    reset_attribute((*it)->getAttribute(terms::mp));
    (*it)->resetStatus();
  }
}

void Player::handleStep()
{
  static int steps = 0;
  steps++;

  if (steps == 4)
  {
    bool playSound = false;

    for (PlayerCharacter* pc : m_party)
    {
      std::vector<StatusEffect*> statusEffects = pc->getStatusEffects();
      for (StatusEffect* effect : statusEffects)
      {
        if (effect->damageType != DAMAGE_NONE)
        {
          effect->applyDamage(pc);

          pc->flash().shake(6, 4);
          playSound = true;
        }
      }
    }

    if (playSound)
    {
      SceneManager::instance().flashScreen(4, sf::Color::Red);

      std::string sound = config::get("SOUND_FIELD_DAMAGE");
      if (sound.size())
      {
        play_sound(sound);
      }
    }

    steps = 0;
  }
}

void Player::rotate(float angle, float speed)
{
  m_isRotating = true;
  m_targetAngle = angle;
  m_rotationInterpolator.set(m_camera.horizontal_angle, m_targetAngle, speed);
}

void Player::initCamera(Direction initDir)
{
  switch (player()->getDirection())
  {
  case DIR_LEFT:
    m_camera.horizontal_angle = -glm::pi<float>() / 2;
    break;
  case DIR_UP:
    m_camera.horizontal_angle = glm::pi<float>() / 4;
    break;
  case DIR_DOWN:
    m_camera.horizontal_angle = glm::pi<float>() / 2 + glm::pi<float>() / 4;
    break;
  case DIR_RIGHT:
    m_camera.horizontal_angle = glm::pi<float>() / 2;
    break;
  default:
    break;
  }
}

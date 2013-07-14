#include <SFML/Window.hpp>

#include <algorithm>
#include <sstream>

#include "SaveLoad.h"
#include "Utility.h"

#include "Direction.h"
#include "Entity.h"
#include "Sprite.h"
#include "PlayerClass.h"
#include "Game.h"
#include "Player.h"

#include "../dep/tinyxml2.h"

using namespace tinyxml2;

Player::Player()
 : m_gold(0),
   m_controlsEnabled(true)
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
  for (auto it = m_playerTrain.begin(); it != m_playerTrain.end(); ++it)
  {
    if (!(*it)->isWalking())
    {
      m_trainCoords[*it].x = (*it)->x;
      m_trainCoords[*it].y = (*it)->y;
    }
  }

  if (m_controlsEnabled)
  {
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
  doc.LoadFile("Resources/Player.xml");

  const XMLElement* root = doc.FirstChildElement("player");
  const XMLElement* start = root->FirstChildElement("start");
  if (start)
  {
    startX = fromString<int>(start->FindAttribute("x")->Value());
    startY = fromString<int>(start->FindAttribute("y")->Value());
    std::string startMap = start->FindAttribute("map")->Value();

    Game::instance().loadNewMap("Resources/Maps/" + startMap);
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

Player* Player::createFromSaveData(std::vector<CharacterData*> charData, std::vector<EntityData*> entData)
{
  Player* player = new Player;

  for (size_t i = 0; i < entData.size(); i++)
  {
    Entity* entity = new Entity(entData[i]->name);
    entity->setPosition(entData[i]->x, entData[i]->y);
    entity->setDirection(entData[i]->dir);
    entity->setWalkSpeed(entData[i]->speed);
    entity->setWalkThrough(entData[i]->walkThrough);

    if (entity->sprite()->getTextureName() != entData[i]->spriteName)
    {
      entity->sprite()->changeTexture(entData[i]->spriteName);
    }

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

void Player::addNewCharacter(const std::string& name, const std::string& className, int x, int y)
{
  PlayerClass pc = player_class_ref(className);

  Entity* entity = new Entity;
  entity->setPosition(x, y);
  entity->setWalkSpeed(0.1f);

  Sprite* sprite = new Sprite;
  sprite->create(pc.texture, pc.textureBlock.x, pc.textureBlock.y);

  entity->setSprite(sprite);

  if (m_playerTrain.size() > 0)
    entity->setWalkThrough(true);

  m_playerTrain.push_back(entity);

  m_party.push_back(PlayerCharacter::create(name, className));
}

void Player::recoverAll()
{
  for (auto it = m_party.begin(); it != m_party.end(); ++it)
  {
    reset_attribute((*it)->getAttribute("hp"));
    reset_attribute((*it)->getAttribute("mp"));
    (*it)->resetStatus();
  }
}

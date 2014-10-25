#include <cmath>
#include <algorithm>
#include <string>

#include <SFML/System.hpp>

#include "SceneManager.h"

#include "Persistent.h"
#include "logger.h"
#include "Map.h"
#include "Config.h"
#include "Player.h"
#include "Message.h"
#include "Game.h"
#include "Sound.h"
#include "Utility.h"
#include "Entity.h"
#include "Direction.h"
#include "Raycaster.h"
#include "Encounter.h"
#include "BattleBackground.h"

#include "SkillTrainer.h"
#include "Shop.h"
#include "Battle.h"

#include "../dep/tinyxml2.h"

using namespace tinyxml2;

Game* Game::theInstance = 0;

Game& Game::instance()
{
  if (theInstance == 0)
  {
    theInstance = new Game;
  }

  return (*theInstance);
}

Game::Game()
 : m_currentMap(0),
   m_player(0),
   m_choiceMenu(0),
   m_transferInProgress(false),
   m_playerMoved(false),
   m_camera(Vec2(4.5f, 4.5f), Vec2(-1, 0), Vec2(0, -0.66f)),
   m_raycaster(new Raycaster(config::RAYCASTER_RES_X, config::RAYCASTER_RES_Y)),

   m_isRotating(false),
   m_angleToRotate(0),
   m_angleInc(0),
   m_accumulatedAngle(0),
   m_rotateDegs(0),

   m_rotKeyDown(false),

   m_minimap(1 + config::GAME_RES_X - 60, 1 + config::GAME_RES_Y - 68, 56, 56),
   m_battleInProgress(false)
{
  m_raycasterBuffer.create(config::RAYCASTER_RES_X, config::RAYCASTER_RES_Y);
  m_texture.create(config::RAYCASTER_RES_X, config::RAYCASTER_RES_Y);
  m_targetTexture.create(config::RAYCASTER_RES_X, config::RAYCASTER_RES_Y);

  // Clear all persistents when a new game is created.
  Persistent<int>::instance().clear();
}

Game::~Game()
{
  delete m_currentMap;
  delete m_player;
  delete m_choiceMenu;

  theInstance = 0;
}

void Game::start(Player* thePlayer)
{
  int startX = 0;
  int startY = 0;

  XMLDocument doc;
  doc.LoadFile(config::res_path("Init.xml").c_str());

  const XMLElement* root = doc.FirstChildElement("init");
  const XMLElement* start = root->FirstChildElement("start");
  if (start)
  {
    startX = fromString<int>(start->FindAttribute("x")->Value());
    startY = fromString<int>(start->FindAttribute("y")->Value());
    std::string startMap = start->FindAttribute("map")->Value();

    loadNewMap("Maps/" + startMap);
  }

  const XMLElement* inventory = root->FirstChildElement("inventory");
  if (inventory)
  {
    for (const XMLElement* element = inventory->FirstChildElement(); element; element = element->NextSiblingElement())
    {
      std::string name = element->FindAttribute("name")->Value();
      int amount = fromString<int>(element->FindAttribute("amount")->Value());

      thePlayer->addItemToInventory(name, amount);
    }
  }

  const XMLElement* goldElem = root->FirstChildElement("gold");
  if (goldElem)
  {
    thePlayer->gainGold(fromString<int>(goldElem->GetText()));
  }

  thePlayer->player()->setPosition(startX, startY);

  setPlayer(thePlayer);
}

void Game::update()
{
  if (m_player)
  {
    m_camera.pos.x = m_player->player()->x + 0.5f;
    m_camera.pos.y = m_player->player()->y + 0.5f;
  }

  if (Message::instance().isVisible())
  {
    Message::instance().update();

    // If it is the absolute last message, pop up the choice menu.
    if (m_choiceMenu && !m_choiceMenu->isVisible() && Message::instance().lastMessage())
    {
      m_choiceMenu->setVisible(true);
    }
  }
  else if (m_menu.isVisible())
  {

  }
  else
  {
    if (m_currentMap && !m_transferInProgress)
      m_currentMap->update();

    m_playerMoved = m_player->player()->isWalking();

    updatePlayer();

    // Only check for warps if the player moved onto the tile this update step.
    if (m_playerMoved && !m_player->player()->isWalking())
    {
      // Update minimap position only when player has finished moving.
      m_minimap.updatePosition(m_currentMap, m_player->player()->x, m_player->player()->y);

      if (!checkWarps() && !checkTraps() && !checkInteractions())
      {
        // Also check encounters if no warps were taken.

        const Encounter* encounter =
            m_currentMap->checkEncounter();
        if (encounter && config::ENCOUNTERS_ENABLED)
        {
          encounter->start();
        }
      }
    }
  }
}

void Game::handleEvent(sf::Event& event)
{
  switch (event.type)
  {
  case sf::Event::KeyPressed:
    handleKeyPress(event.key.code);

    break;
  default:
    break;
  }
}

void Game::handleKeyPress(sf::Keyboard::Key key)
{
  if (key == sf::Keyboard::Space)
  {
    if (Message::instance().isVisible() && Message::instance().isWaitingForKey())
    {
      // CHoice menu when message box open: handle confirm and close message box.
      if (m_choiceMenu && m_choiceMenu->isVisible())
      {
        m_choiceMenu->handleConfirm();

        closeChoiceMenu();
      }

      Message::instance().nextPage();
    }
    else if (Message::instance().isVisible())
    {
      Message::instance().flush();
    }
    else if (!Message::instance().isVisible())
    {
      if (m_menu.isVisible())
      {
        m_menu.handleConfirm();
      }
      else if (m_choiceMenu && m_choiceMenu->isVisible())
      {
        // ChoiceMenu when no message box is open.

        m_choiceMenu->handleConfirm();

        closeChoiceMenu();
      }
      else if (m_player->isControlsEnabled())
      {
        for (auto it = m_currentMap->getEntities().begin(); it != m_currentMap->getEntities().end(); ++it)
        {
          if ((*it)->canInteractWith(m_player->player()))
          {
            (*it)->interact(m_player->player());
          }
        }
      }
    }
  }
  else if (key == sf::Keyboard::Escape)
  {
    if (!Message::instance().isVisible() && m_player->isControlsEnabled())
    {
      if (!m_player->player()->isWalking() && !Message::instance().isVisible() && !m_menu.isVisible())
      {
        m_menu.open();
      }
      else
      {
        m_menu.handleEscape();
      }
    }
  }

  if (m_menu.isVisible() && !Message::instance().isVisible())
  {
    if (key == sf::Keyboard::Down) m_menu.moveArrow(DIR_DOWN);
    else if (key == sf::Keyboard::Up) m_menu.moveArrow(DIR_UP);
    else if (key == sf::Keyboard::Right) m_menu.moveArrow(DIR_RIGHT);
    else if (key == sf::Keyboard::Down) m_menu.moveArrow(DIR_DOWN);
  }
  else if (m_choiceMenu && m_choiceMenu->isVisible())
  {
    if (key == sf::Keyboard::Down) m_choiceMenu->moveArrow(DIR_DOWN);
    else if (key == sf::Keyboard::Up) m_choiceMenu->moveArrow(DIR_UP);
    else if (key == sf::Keyboard::Right) m_choiceMenu->moveArrow(DIR_RIGHT);
    else if (key == sf::Keyboard::Down) m_choiceMenu->moveArrow(DIR_DOWN);
  }

  if (key == sf::Keyboard::B && config::get("DEBUG_MODE") == "true")
  {
    std::vector<std::string> monsters = split_string(config::get("DEBUG_BATTLE"), ',');
    startBattle(monsters);
  }
  else if (key == sf::Keyboard::S && config::get("DEBUG_MODE") == "true")
  {
    std::vector<std::string> items = split_string(config::get("DEBUG_SHOP"), ',');
    openShop(items);
  }
}

void Game::draw(sf::RenderTarget& target)
{
  for (int y = 0; y < config::RAYCASTER_RES_Y; y++)
  {
    for (int x = 0; x < config::RAYCASTER_RES_X; x++)
    {
      m_raycasterBuffer.setPixel(x, y, sf::Color::Black);
    }
  }

  m_raycaster->raycast(&m_camera, m_raycasterBuffer, m_player->player()->getDirection());
  m_texture.loadFromImage(m_raycasterBuffer);

  sf::Sprite sprite(m_texture);
  m_targetTexture.draw(sprite);
  m_targetTexture.display();

  sprite.setTexture(m_targetTexture.getTexture());
  sprite.setPosition(0, 0);
  target.draw(sprite);

  ////////////////////////////////////////////////////////////////////////////
  // Draw faces for characters under raycaster view.
  ////////////////////////////////////////////////////////////////////////////
  drawParty(target);

  m_minimap.draw(target);

  if (m_menu.isVisible())
  {
    m_menu.draw(target, 0, 0);
  }

  if (m_currentMap)
  {
    if (const sf::Texture* background = m_currentMap->getBackground())
    {
      sf::Sprite sprite;
      sprite.setTexture(*background);
      target.draw(sprite);
    }
  }

  if (Message::instance().isVisible())
  {
    Message::instance().draw(target);
  }

  if (m_choiceMenu && m_choiceMenu->isVisible())
  {
    m_choiceMenu->draw(target, 0, config::GAME_RES_Y - 68 - m_choiceMenu->getHeight());
  }
}

void Game::drawParty(sf::RenderTarget& target) const
{
  int delta = config::GAME_RES_Y - config::RAYCASTER_RES_Y;
  int yPos = config::RAYCASTER_RES_Y + (delta / 2) - 24;

  auto party = m_player->getParty();
  int partyPosX = 16;
  for (size_t i = 0; i < party.size(); i++)
  {
    PlayerCharacter* character = party[i];

    int xPos;

    xPos = partyPosX + i * 44;

    character->draw(target, xPos, yPos);

    float hpPercent = (float)character->getAttribute("hp").current / (float)character->getAttribute("hp").max;
    float mpPercent = (float)character->getAttribute("mp").current / (float)character->getAttribute("mp").max;

    //////////////////////////////////////////////////////////////////////////
    // Draw darker rectangles to display lost HP/MP.
    //////////////////////////////////////////////////////////////////////////
    sf::RectangleShape hpFullRect;
    sf::RectangleShape mpFullRect;
    hpFullRect.setFillColor(sf::Color(127, 0, 0));
    mpFullRect.setFillColor(sf::Color(0, 0, 127));

    hpFullRect.setSize(sf::Vector2f(32, 4));
    mpFullRect.setSize(sf::Vector2f(32, 4));

    hpFullRect.setPosition(xPos, yPos + 32);
    mpFullRect.setPosition(xPos, yPos + 36);

    target.draw(hpFullRect);
    target.draw(mpFullRect);

    //////////////////////////////////////////////////////////////////////////
    // Draw the lighter regular rectangles.
    //////////////////////////////////////////////////////////////////////////
    sf::RectangleShape hpRect;
    sf::RectangleShape mpRect;
    hpRect.setFillColor(sf::Color::Red);
    mpRect.setFillColor(sf::Color::Blue);

    hpRect.setSize(sf::Vector2f(32.0f * hpPercent, 4));
    mpRect.setSize(sf::Vector2f(32.0f * mpPercent, 4));

    hpRect.setPosition(xPos, yPos + 32);
    mpRect.setPosition(xPos, yPos + 36);

    target.draw(hpRect);
    target.draw(mpRect);
  }
}

void Game::updatePlayer()
{
  if (m_player)
  {
    m_player->update();

    if (!m_player->player()->isWalking() && !m_isRotating && m_player->isControlsEnabled())
    {
      if (!m_rotKeyDown && sf::Keyboard::isKeyPressed(sf::Keyboard::Left))
      {
        startRotate(-90, -10);
        m_player->player()->turnLeft();

        m_rotKeyDown = true;
      }
      else if (!m_rotKeyDown && sf::Keyboard::isKeyPressed(sf::Keyboard::Right))
      {
        startRotate(90, 10);
        m_player->player()->turnRight();

        m_rotKeyDown = true;
      }

      if (sf::Keyboard::isKeyPressed(sf::Keyboard::Left) || sf::Keyboard::isKeyPressed(sf::Keyboard::Right))
      {
        m_rotKeyDown = true;
      }
      else
      {
        m_rotKeyDown = false;
      }
    }

    if (m_isRotating)
    {
      execRotate();
    }
  }
}

bool Game::checkWarps()
{
  if (m_player && m_currentMap &&
      !m_player->player()->isWalking() &&
      m_currentMap->warpAt(m_player->player()->x, m_player->player()->y))
  {
    const Warp* warp = m_currentMap->getWarpAt(m_player->player()->x, m_player->player()->y);

    play_sound(config::get("SOUND_MOVEMENT"));

    prepareTransfer(warp->destMap, warp->dstX, warp->dstY);

    return true;
  }

  return false;
}

bool Game::checkTraps()
{
  if (m_player && m_currentMap &&
      !m_player->player()->isWalking() &&
      m_currentMap->trapAt(m_player->player()->x, m_player->player()->y))
  {
    const Trap* trap = m_currentMap->getTrapAt(m_player->player()->x, m_player->player()->y);

    trap->checkTrap();
    m_currentMap->disableTrap(trap);

    return true;
  }

  return false;
}

bool Game::checkInteractions()
{
  auto entities = m_currentMap->getEntities();

  if (m_player && m_currentMap &&
      !m_player->player()->isWalking())
  {
    Entity* entity = 0;

    for (auto it = entities.begin(); it != entities.end(); ++it)
    {
      if ((int)(*it)->x == (int)m_player->player()->x &&
          (int)(*it)->y == (int)m_player->player()->y)
      {
        entity = *it;
        break;
      }
    }

    if (entity)
    {
      entity->interact(m_player->player());

      return true;
    }
  }

  return false;
}

void Game::prepareTransfer(const std::string& targetMap, int x, int y)
{
  SceneManager::instance().fadeOut(32);

  m_currentWarp.dstX = x;
  m_currentWarp.dstY = y;
  m_currentWarp.destMap = targetMap;
  m_transferInProgress = true;
}

void Game::transferPlayer(const std::string& targetMap, int x, int y)
{
  if (targetMap != m_currentMap->getName())
  {
    loadNewMap(targetMap);
  }

  m_player->transfer(x, y);

  // Also update minimap when player has transfered.
  m_minimap.updatePosition(m_currentMap, x, y);
}

void Game::playMusic(const std::string& music)
{
  if (music != m_currentMusicName)
  {
    m_currentMusicName = music;

    m_currentMusic.stop();
    m_currentMusic.openFromFile(config::res_path("Music/" + music));
    m_currentMusic.setLoop(true);
    m_currentMusic.setVolume(75);
    m_currentMusic.play();
  }
}

void Game::openChoiceMenu(const std::vector<std::string>& choices)
{
  m_choiceMenu = new ChoiceMenu;
  m_choiceMenu->setVisible(false);
  for (auto it = choices.begin(); it != choices.end(); ++it)
  {
    m_choiceMenu->addEntry(*it);
  }
}

void Game::closeChoiceMenu()
{
  delete m_choiceMenu;
  m_choiceMenu = 0;
}

void Game::loadNewMap(const std::string& file)
{
  delete m_currentMap;
  m_currentMap = Map::loadTiledFile(file);

  if (!m_currentMap)
  {
    TRACE("Loading map failed! Quitting game...");
    SceneManager::instance().close();
  }

  m_raycaster->setTilemap(m_currentMap);
  m_raycaster->clearEntities();
  for (auto it = m_currentMap->getEntities().begin(); it != m_currentMap->getEntities().end(); ++it)
  {
    if ((*it)->sprite())
    {
      m_raycaster->addEntity(*it);
    }
  }

  if (!m_currentMap->getMusic().empty())
  {
    playMusic(m_currentMap->getMusic());
  }
}

void Game::startBattle(const std::vector<std::string>& monsters, bool canEscape, const std::string& music)
{
  if (music.size())
  {
    m_savedBattleMusic = config::get("MUSIC_BATTLE");
    config::set("MUSIC_BATTLE", "Music/" + music);
  }

  std::string startBattleSound = config::get("SOUND_BATTLE");
  if (startBattleSound.size())
  {
    play_sound(startBattleSound);
  }

  m_currentMusic.pause();

  Message::instance().setIsQuiet(true);

  std::string traceString;

  std::vector<Character*> monsterChars;
  for (auto it = monsters.begin(); it != monsters.end(); ++it)
  {
    traceString += (*it) + " ";
    monsterChars.push_back(Character::createMonster(*it));
  }

  TRACE("Starting combat with: %s", traceString.c_str());

  BattleBackground* battleBg = new BattleBackground(
      m_currentMap,
      m_player->player()->x,
      m_player->player()->y,
      m_player->player()->getDirection());

  Battle* battle = new Battle(monsterChars);
  battle->setBattleBackground(battleBg);
  battle->start(canEscape);

  m_battleInProgress = true;

  SceneManager::instance().addScene(battle);
}

void Game::postBattle()
{
  Message::instance().setIsQuiet(false);

  if (m_savedBattleMusic.size())
  {
    config::set("MUSIC_BATTLE", m_savedBattleMusic);
    m_savedBattleMusic = "";
  }

  m_currentMusic.play();
  m_battleInProgress = false;

  SceneManager::instance().fadeIn(32);
}

void Game::preFade(FadeType fadeType)
{
  if (fadeType == FADE_OUT)
  {
    m_player->setControlsEnabled(false);
  }
}

void Game::postFade(FadeType fadeType)
{
  if (fadeType == FADE_IN)
  {
    m_player->setControlsEnabled(true);
  }
  else
  {
    if (m_transferInProgress)
    {
      std::string warpTargetName = getWarpTargetName(m_currentWarp);

      transferPlayer(warpTargetName, m_currentWarp.dstX, m_currentWarp.dstY);

      m_transferInProgress = false;

      SceneManager::instance().fadeIn(32);
    }
  }
}

void Game::openShop(const std::vector<std::string>& items)
{
  Shop* shop = new Shop(items);
  SceneManager::instance().addScene(shop);
}

void Game::openSkillTrainer(const std::vector<std::string>& skills)
{
  SkillTrainer* skillTrainer = new SkillTrainer(skills);
  SceneManager::instance().addScene(skillTrainer);
}

void Game::startRotate(int angle, int angleInc)
{
  m_isRotating = true;
  m_angleToRotate = angle;
  m_angleInc = angleInc;
  m_accumulatedAngle = 0;
  m_rotateDegs = PI_F * (float)angleInc / 180.0f;
}

void Game::execRotate()
{
  m_camera.rotate(m_rotateDegs);

  m_accumulatedAngle += m_angleInc;

  if (abs(m_accumulatedAngle) >= abs(m_angleToRotate))
  {
    m_isRotating = false;
  }
}

void Game::setPlayer(Player* player)
{
  m_player = player;
  Direction playerDir = m_player->player()->getDirection();

  if (playerDir == DIR_UP)
  {
    m_camera.rotate(deg2rad(90));
  }
  else if (playerDir == DIR_DOWN)
  {
    m_camera.rotate(deg2rad(-90));
  }
  else if (playerDir == DIR_RIGHT)
  {
    m_camera.rotate(deg2rad(180));
  }

  // So it shows up at beginning of game.
  m_minimap.updatePosition(m_currentMap, m_player->player()->x, m_player->player()->y);
}

#include <algorithm>
#include <string>

#include <SFML/System.hpp>

#include "SceneManager.h"

#include "logger.h"
#include "Map.h"
#include "Config.h"
#include "Player.h"
#include "Message.h"
#include "Game.h"
#include "Sound.h"

#include "Battle.h"

Game& Game::instance()
{
  static Game game;
  return game;
}

Game::Game()
 : m_currentMap(0),
   m_player(0),
   m_choiceMenu(0),
   m_fade(FADE_NONE),
   m_fadeCounter(0),
   m_fadeDuration(0),
   m_currentWarp(0),
   m_playerMoved(false)
{

}

Game::~Game()
{
  delete m_currentMap;
  delete m_player;
  delete m_choiceMenu;
}

void Game::update()
{
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
    if (m_currentMap)
      m_currentMap->update();

    m_playerMoved = m_player->player()->isWalking();

    updatePlayer();

    // Only check for warps if the player moved onto the tile this update step.
    if (m_playerMoved && !m_player->player()->isWalking())
    {
      if (!checkWarps())
      {
        // Also check encounters if no warps were taken.

        std::vector<std::string> monsters =
            m_currentMap->checkEncounter(m_player->player()->x, m_player->player()->y);
        if (!monsters.empty())
        {
          startBattle(monsters);
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

  if (key == sf::Keyboard::B)
  {
    std::vector<std::string> monsters = {"Skelington", "Skelington", "Skelington"};
    startBattle(monsters);
  }
}

void Game::draw(sf::RenderTarget& target)
{
  if (m_currentMap)
    m_currentMap->draw(target, m_view);

  if (m_player)
    m_player->draw(target, m_view);

  if (m_menu.isVisible())
  {
    m_menu.draw(target, 0, 0);
  }

  if (Message::instance().isVisible())
  {
    Message::instance().draw(target);
  }

  if (m_choiceMenu && m_choiceMenu->isVisible())
  {
    m_choiceMenu->draw(target, 0, config::GAME_RES_Y - 48 - m_choiceMenu->getHeight());
  }
}

void Game::updatePlayer()
{
  if (m_player)
  {
    m_player->update();

    m_view.x = m_player->player()->getRealX() - config::GAME_RES_X / 2;
    m_view.y = m_player->player()->getRealY() - config::GAME_RES_Y / 2;

    m_view.x = std::min(m_currentMap->getWidth() * config::TILE_W - config::GAME_RES_X, m_view.x);
    m_view.y = std::min(m_currentMap->getHeight() * config::TILE_H - config::GAME_RES_Y, m_view.y);
    m_view.x = std::max(0, m_view.x);
    m_view.y = std::max(0, m_view.y);
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
    SceneManager::instance().fadeOut(32);

    m_currentWarp = warp;

    return true;
  }

  return false;
}

void Game::transferPlayer(const std::string& targetMap, int x, int y)
{
  if (targetMap != m_currentMap->getName())
  {
    loadNewMap(targetMap);
  }

  m_player->transfer(x, y);
}

void Game::playMusic(const std::string& music)
{
  if (music != m_currentMusicName)
  {
    m_currentMusicName = music;

    m_currentMusic.stop();
    m_currentMusic.openFromFile("Resources/Music/" + music);
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

  if (!m_currentMap->getMusic().empty())
  {
    playMusic(m_currentMap->getMusic());
  }
}

void Game::startBattle(const std::vector<std::string>& monsters)
{
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

  Battle* battle = new Battle(monsterChars);
  battle->start();

  SceneManager::instance().addScene(battle);
}

void Game::postBattle()
{
  Message::instance().setIsQuiet(false);

  m_currentMusic.play();
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
    if (m_currentWarp)
    {
      std::string warpTargetName = getWarpTargetName(*m_currentWarp);

      transferPlayer(warpTargetName, m_currentWarp->dstX, m_currentWarp->dstY);

      m_currentWarp = 0;

      SceneManager::instance().fadeIn(32);
    }
  }
}

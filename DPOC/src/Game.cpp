#include <algorithm>
#include <string>

#include <SFML/System.hpp>

#include "Map.h"
#include "Config.h"
#include "Player.h"
#include "Message.h"
#include "Game.h"

#include "Battle.h"

Game& Game::instance()
{
  static Game game;
  return game;
}

Game::Game()
 : m_currentMap(0),
   m_player(Player::create(4, 4))
{
  m_window.create(sf::VideoMode(config::GAME_RES_X, config::GAME_RES_Y), "DPOC");
  loadNewMap("Resources/test.map");
}

Game::~Game()
{
  m_window.close();

  delete m_currentMap;
  delete m_player;
}

void Game::run()
{
  sf::Clock clock;
  int timerThen = clock.restart().asMilliseconds();

  bool playerMoved = false;

  while (m_window.isOpen())
  {
    int timerNow = clock.getElapsedTime().asMilliseconds();

    while (timerThen < timerNow)
    {
      pollEvents();

      if (Message::instance().isVisible())
      {
        Message::instance().update();
      }
      else if (m_menu.isVisible())
      {

      }
      else
      {
        if (m_currentMap)
          m_currentMap->update();

        playerMoved = m_player->player()->isWalking();

        updatePlayer();

        // Only check for warps if the player moved onto the tile this update step.
        if (playerMoved && !m_player->player()->isWalking())
          checkWarps();
      }

      timerThen += 1000 / config::FPS;
    }

    draw();

    sf::sleep(sf::milliseconds(timerThen - timerNow));
  }
}

void Game::pollEvents()
{
  sf::Event event;

  while (m_window.pollEvent(event))
  {
    switch (event.type)
    {
    case sf::Event::Closed:
      m_window.close();
      break;
    case sf::Event::KeyPressed:
      handleKeyPress(event.key.code);

      break;
    default:
      break;
    }
  }
}

void Game::handleKeyPress(sf::Keyboard::Key key)
{
  if (key == sf::Keyboard::Space)
  {
    if (Message::instance().isVisible() && Message::instance().isWaitingForKey())
    {
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
      else
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
    if (!Message::instance().isVisible())
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

  if (key == sf::Keyboard::B)
  {
    Battle battle(m_window,
      {
        Character::createMonster("Skelington"),
        Character::createMonster("Skelington"),
        Character::createMonster("Skelington")
      });
    battle.start();
  }
}

void Game::draw()
{
  m_window.clear();

  if (m_currentMap)
    m_currentMap->draw(m_window, m_view);

  if (m_player)
    m_player->draw(m_window, m_view);

  if (m_menu.isVisible())
  {
    m_menu.draw(m_window, 0, 0);
  }

  if (Message::instance().isVisible())
  {
    Message::instance().draw(m_window);
  }

  m_window.display();
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

void Game::checkWarps()
{
  if (m_player && m_currentMap &&
      !m_player->player()->isWalking() &&
      m_currentMap->warpAt(m_player->player()->x, m_player->player()->y))
  {
    const Warp* warp = m_currentMap->getWarpAt(m_player->player()->x, m_player->player()->y);
    std::string warpTargetName = getWarpTargetName(*warp);

    transferPlayer(warpTargetName, warp->dstX, warp->dstY);
  }
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
    m_currentMusic.openFromFile("Resources/" + music);
    m_currentMusic.setLoop(true);
    m_currentMusic.play();
  }
}

void Game::loadNewMap(const std::string& file)
{
  delete m_currentMap;
  m_currentMap = Map::loadFromFile(file);

  if (m_currentMap->getMusic() != "<none>")
  {
    playMusic(m_currentMap->getMusic());
  }
}

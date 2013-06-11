#include <algorithm>
#include <string>

#include <SFML/System.hpp>

#include "Map.h"
#include "Config.h"
#include "Player.h"
#include "Game.h"

Game& Game::instance()
{
  static Game game;
  return game;
}

Game::Game()
 : m_currentMap(Map::loadFromFile("Resources/test.map")),
   m_player(Player::create(4, 4))
{
  m_window.create(sf::VideoMode(config::GAME_RES_X, config::GAME_RES_Y), "DPOC");
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

  while (m_window.isOpen())
  {
    int timerNow = clock.getElapsedTime().asMilliseconds();

    while (timerThen < timerNow)
    {
      pollEvents();

      if (m_currentMap)
        m_currentMap->update();

      updatePlayer();

      checkWarps();

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
    default:
      break;
    }
  }
}

void Game::draw()
{
  m_window.clear();

  if (m_currentMap)
    m_currentMap->draw(m_window, m_view);

  if (m_player)
    m_player->draw(m_window, m_view);

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
    delete m_currentMap;
    m_currentMap = Map::loadFromFile(targetMap);
  }

  m_player->player()->x = x;
  m_player->player()->y = y;
}

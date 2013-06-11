#include <SFML/System.hpp>

#include "Config.h"
#include "Game.h"

Game::Game()
{
  m_window.create(sf::VideoMode(256, 240), "DPOC");
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

  }
}

void Game::draw()
{
  m_window.clear();

  m_window.display();
}

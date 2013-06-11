#include "Game.h"

Game::Game()
{
  m_window.create(sf::VideoMode(256, 240), "DPOC");
}

void Game::run()
{
  while (m_window.isOpen())
  {
    pollEvents();
    draw();
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

#include "Config.h"
#include "Battle.h"

Battle::Battle(sf::RenderWindow& window, const std::vector<Character*>& monsters)
 : m_battleOngoing(false),
   m_battleMenu(monsters),
   m_window(window)
{

}

void Battle::start()
{
  sf::Clock clock;
  int timerThen = clock.restart().asMilliseconds();

  m_battleOngoing = true;

  while (m_window.isOpen() && m_battleOngoing)
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

void Battle::pollEvents()
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

void Battle::handleKeyPress(sf::Keyboard::Key key)
{
  if (key == sf::Keyboard::Up) m_battleMenu.moveArrow(DIR_UP);
  else if (key == sf::Keyboard::Down) m_battleMenu.moveArrow(DIR_DOWN);
  else if (key == sf::Keyboard::Left) m_battleMenu.moveArrow(DIR_LEFT);
  else if (key == sf::Keyboard::Right) m_battleMenu.moveArrow(DIR_RIGHT);
  else if (key == sf::Keyboard::Space) m_battleMenu.handleConfirm();
  else if (key == sf::Keyboard::Escape) m_battleMenu.handleEscape();
}

void Battle::draw()
{
  m_window.clear();

  m_battleMenu.draw(m_window, 0, 152);

  m_window.display();
}

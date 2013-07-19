#include <string>

#include "SceneManager.h"
#include "Config.h"
#include "Cache.h"

#include "SaveLoad.h"
#include "Game.h"
#include "Player.h"

#include "TitleScreen.h"

void TitleMenu::handleConfirm()
{
  std::string choice = getCurrentMenuChoice();

  if (choice == "New Game")
  {
    setVisible(false);

    Game::instance().setPlayer(Player::create());
    SceneManager::instance().fadeOut(32);
  }
  else if (choice == "Load Game")
  {
    setVisible(false);

    load_game("TestSave.xml");
    SceneManager::instance().fadeOut(32);
  }
  else if (choice == "Exit")
  {
    SceneManager::instance().close();
  }
}

TitleScreen::TitleScreen()
{
  m_menu.addEntry("New Game");
  m_menu.addEntry("Load Game");
  m_menu.addEntry("Exit");
  m_menu.setVisible(true);

  m_titleTexture = cache::loadTexture("Resources/Title/TitleScreen.png");
}

TitleScreen::~TitleScreen()
{
  cache::releaseTexture(m_titleTexture);
}

void TitleScreen::update()
{
}

void TitleScreen::draw(sf::RenderTarget& target)
{
  if (m_titleTexture)
  {
    sf::Sprite sprite;
    sprite.setTexture(*m_titleTexture);
    target.draw(sprite);
  }

  m_menu.draw(target, config::GAME_RES_X / 2 - m_menu.getWidth() / 2, config::GAME_RES_Y / 2);
}

void TitleScreen::handleEvent(sf::Event& event)
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

void TitleScreen::handleKeyPress(sf::Keyboard::Key key)
{
  if (m_menu.isVisible())
  {
    if (key == sf::Keyboard::Space)
    {
      m_menu.handleConfirm();
    }

    if (key == sf::Keyboard::Down) m_menu.moveArrow(DIR_DOWN);
    else if (key == sf::Keyboard::Up) m_menu.moveArrow(DIR_UP);
    else if (key == sf::Keyboard::Right) m_menu.moveArrow(DIR_RIGHT);
    else if (key == sf::Keyboard::Down) m_menu.moveArrow(DIR_DOWN);
  }
}

void TitleScreen::postFade(FadeType fadeType)
{
  if (fadeType == FADE_OUT)
  {
    //close();
    SceneManager::instance().fadeIn(32);
    SceneManager::instance().addScene(&Game::instance());
  }
}

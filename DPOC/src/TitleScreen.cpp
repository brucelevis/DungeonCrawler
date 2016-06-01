#include <string>

#include "SceneManager.h"
#include "Config.h"
#include "Cache.h"

#include "SaveLoad.h"
#include "Game.h"
#include "Player.h"
#include "Message.h"

#include "CharGen.h"
#include "TitleScreen.h"

TitleMenu::TitleMenu()
 : m_saveMenu(0)
{

}

TitleMenu::~TitleMenu()
{
  delete m_saveMenu;
}

void TitleMenu::handleConfirm()
{
  if (m_saveMenu)
  {
    m_saveMenu->handleConfirm();

    if (!m_saveMenu->isVisible())
    {
      delete m_saveMenu;
      m_saveMenu = 0;

      setVisible(false);
      SceneManager::instance().fadeOut(32);
    }
  }
  else
  {
    std::string choice = getCurrentMenuChoice();

    if (choice == "New Game")
    {
      setVisible(false);

      //Game::instance().setPlayer(Player::create());
      SceneManager::instance().fadeOut(32);
    }
    else if (choice == "Load Game")
    {
      m_saveMenu = new SaveMenu(SaveMenu::LOAD);
      m_saveMenu->setVisible(true);
    }
    else if (choice == "Exit")
    {
      SceneManager::instance().close();
    }
  }
}

void TitleMenu::moveArrow(Direction dir)
{
  if (m_saveMenu)
  {
    m_saveMenu->moveArrow(dir);
  }
  else
  {
    Menu::moveArrow(dir);
  }
}

void TitleMenu::handleEscape()
{
  if (m_saveMenu)
  {
    m_saveMenu->handleEscape();

    if (!m_saveMenu->isVisible())
    {
      delete m_saveMenu;
      m_saveMenu = 0;
    }
  }
}

void TitleMenu::draw(sf::RenderTarget& target, int x, int y)
{
  Menu::draw(target, x, y);

  if (m_saveMenu)
  {
    m_saveMenu->draw(target, config::GAME_RES_X / 2 - m_saveMenu->getWidth() / 2,
        config::GAME_RES_Y / 2 - m_saveMenu->getHeight() / 2);
  }
}

///////////////////////////////////////////////////////////////////////////////

TitleScreen::TitleScreen()
{
  m_menu.addEntry("New Game");
  m_menu.addEntry("Load Game");
  m_menu.addEntry("Exit");
  m_menu.setVisible(true);

  m_titleTexture = cache::loadTexture("Title/TitleScreen.png");
  m_titleMusic.openFromFile(config::res_path(config::get("MUSIC_TITLE")));
  m_titleMusic.play();
}

TitleScreen::~TitleScreen()
{
  cache::releaseTexture(m_titleTexture);
}

void TitleScreen::update()
{
}

void TitleScreen::draw(sf::RenderTexture& target)
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
    else if (key == sf::Keyboard::Escape)
    {
      m_menu.handleEscape();
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
    m_titleMusic.stop();

    SceneManager::instance().fadeIn(32);

    if (m_menu.getCurrentMenuChoice() == "New Game")
    {
      SceneManager::instance().addScene(new CharGen);
    }
    else if (m_menu.getCurrentMenuChoice() == "Load Game")
    {
      SceneManager::instance().addScene(&Game::instance());
    }
  }
  else if (fadeType == FADE_IN)
  {
    m_menu.setVisible(true);
    Message::instance().clear();

    m_titleMusic.play();
  }
}

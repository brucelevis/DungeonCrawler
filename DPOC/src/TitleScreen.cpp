#include <string>
#include <functional>

#include "logger.h"
#include "SceneManager.h"
#include "Config.h"
#include "Cache.h"

#include "SaveLoad.h"
#include "Game.h"
#include "Player.h"
#include "Message.h"

#include "CharGen.h"
#include "Scenario.h"

#include "Menu_TitleMenu.h"
#include "TitleScreen.h"

TitleScreen::TitleScreen()
  : m_selectedAction(TitleMenu::NONE)
{
  createTitleMenu();

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

void TitleScreen::draw(sf::RenderTarget& target)
{
  if (m_titleTexture)
  {
    sf::Sprite sprite;
    sprite.setTexture(*m_titleTexture);
    target.draw(sprite);
  }
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

void TitleScreen::handleTitleOption(TitleMenu::Action action)
{
  SceneManager::instance().fadeOut(32);
  m_selectedAction = action;
}

void TitleScreen::handleKeyPress(sf::Keyboard::Key key)
{
}

void TitleScreen::postFade(FadeType fadeType)
{
  if (fadeType == FADE_OUT)
  {
    //close();
    m_titleMusic.stop();

    SceneManager::instance().fadeIn(32);

    auto guiStack = SceneManager::instance().getGuiStack();
    guiStack->findWidget<TitleMenu>()->close();

    switch (m_selectedAction)
    {
    case TitleMenu::NEW_GAME:
      if (Scenario::instance().useCharGen())
      {
        SceneManager::instance().addScene(new CharGen);
      }
      else
      {
        Game::instance().start(Player::create());
        SceneManager::instance().addScene(&Game::instance());
      }
      break;
    case TitleMenu::LOAD_GAME:
      SceneManager::instance().addScene(&Game::instance());
      break;
    case TitleMenu::EXIT_GAME:
      SceneManager::instance().close();
      break;
    default:
      TRACE("ERROR: Should never get here");
      break;
    }
  }
  else if (fadeType == FADE_IN)
  {
    createTitleMenu();
    Message::instance().clear();

    m_titleMusic.play();
  }
}

void TitleScreen::createTitleMenu()
{
  SceneManager::instance().getGuiStack()->addWidget<TitleMenu>(std::bind(&TitleScreen::handleTitleOption, this, std::placeholders::_1));
}

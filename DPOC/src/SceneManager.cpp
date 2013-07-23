#include <algorithm>

#include <SFML/System.hpp>

#include "Config.h"
#include "Utility.h"
#include "SceneManager.h"

SceneManager& SceneManager::instance()
{
  static SceneManager mgr;
  return mgr;
}

SceneManager::SceneManager()
 : m_shakeCounter(0),
   m_shakeStrengthX(0),
   m_shakeStrengthY(0),
   m_fade(Scene::Scene::FADE_NONE),
   m_fadeCounter(0),
   m_fadeDuration(0)
{
}

void SceneManager::create()
{
  m_window.create(sf::VideoMode(config::GAME_RES_X*2, config::GAME_RES_Y*2), "DPOC");
  m_window.setKeyRepeatEnabled(false);

  m_targetTexture.create(config::GAME_RES_X, config::GAME_RES_Y);
}

void SceneManager::run()
{
  sf::Clock clock;
  int timerThen = clock.restart().asMilliseconds();

  while (m_window.isOpen())
  {
    int timerNow = clock.getElapsedTime().asMilliseconds();

    while (timerThen < timerNow)
    {
      processFade();

      if (m_shakeCounter > 0)
      {
        m_shakeCounter--;
      }

      pollEvents();

      if (m_scenes.size() > 0)
      {
        m_scenes.back()->update();
      }

      cleanUp();

      timerThen += 1000 / config::FPS;
    }

    draw();

    sf::sleep(sf::milliseconds(timerThen - timerNow));
  }
}

void SceneManager::cleanUp()
{
  // Clean up closed scenes.
  auto removeBegin = std::remove_if(m_scenes.begin(), m_scenes.end(),
      [=](Scene* scene) { return scene->isClosed(); });

  for (auto it = removeBegin; it != m_scenes.end(); ++it)
  {
    delete *it;
  }

  m_scenes.erase(removeBegin, m_scenes.end());
}

void SceneManager::draw()
{
  m_window.clear();
  m_targetTexture.clear();

  // Because "clear" doesn't clear...
  sf::RectangleShape clearRect;
  clearRect.setFillColor(sf::Color::Black);
  clearRect.setSize(sf::Vector2f(m_targetTexture.getSize().x, m_targetTexture.getSize().y));
  m_targetTexture.draw(clearRect);

  if (m_scenes.size() > 0)
  {
    m_scenes.back()->draw(m_targetTexture);
  }

  m_targetTexture.display();

  sf::Sprite sprite;
  sprite.setTexture(m_targetTexture.getTexture());
  sprite.setScale(sf::Vector2f(2, 2));
  sprite.setPosition(0, 0);

  if (m_shakeCounter > 0)
  {
    sprite.setPosition(random_range(-m_shakeStrengthX, m_shakeStrengthX),
        random_range(-m_shakeStrengthY, m_shakeStrengthY));
  }

  if (m_fade != Scene::FADE_NONE)
  {
    float percent = (float)m_fadeCounter / (float)m_fadeDuration;

    float opacity = 255;
    if (m_fade == Scene::FADE_OUT)
    {
      opacity = 255.0f * percent;
    }
    else if (m_fade == Scene::FADE_IN)
    {
      opacity = 255.0f - (255.0f * percent);
    }

    sprite.setColor(sf::Color(255, 255, 255, opacity));
  }

  m_window.draw(sprite);
  m_window.display();
}

void SceneManager::shakeScreen(int duration, int shakeStrengthX, int shakeStrengthY)
{
  m_shakeCounter = duration;
  m_shakeStrengthX = shakeStrengthX;
  m_shakeStrengthY = shakeStrengthY;
}

void SceneManager::fadeIn(int duration)
{
  m_fade = Scene::FADE_IN;
  m_fadeCounter = duration;
  m_fadeDuration = duration;

  if (m_scenes.size() > 0)
  {
    m_scenes.back()->preFade(m_fade);
  }
}

void SceneManager::fadeOut(int duration)
{
  m_fade = Scene::FADE_OUT;
  m_fadeCounter = duration;
  m_fadeDuration = duration;

  if (m_scenes.size() > 0)
  {
    m_scenes.back()->preFade(m_fade);
  }
}

void SceneManager::processFade()
{
  if (m_fadeCounter > 0)
  {
    m_fadeCounter--;
    if (m_fadeCounter == 0 && m_fade == Scene::FADE_IN)
    {
      m_fade = Scene::FADE_NONE;

      if (m_scenes.size() > 0)
      {
        m_scenes.back()->postFade(Scene::FADE_IN);
      }
    }
    else if (m_fadeCounter == 0 && m_fade == Scene::FADE_OUT)
    {
      m_fade = Scene::FADE_NONE;

      if (m_scenes.size() > 0)
      {
        m_scenes.back()->postFade(Scene::FADE_OUT);
      }
    }
  }
}

void SceneManager::close()
{
  m_window.close();
}

void SceneManager::pollEvents()
{
  sf::Event event;

  while (m_window.pollEvent(event))
  {
    switch (event.type)
    {
    case sf::Event::Closed:
      close();
      break;
    default:
      if (m_scenes.size() > 0)
      {
        m_scenes.back()->handleEvent(event);
      }
      break;
    }
  }
}

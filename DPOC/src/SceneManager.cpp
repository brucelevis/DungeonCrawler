#include <algorithm>

#include <SFML/System.hpp>

#include "Picture.h"
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
   m_fadeDuration(0),
   m_flashDuration(0),
   m_flashCounter(0)
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

      if (m_flashCounter > 0)
      {
        m_flashCounter--;
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
  for (auto it = m_scenes.begin(); it != m_scenes.end();)
  {
    if ((*it)->isClosed())
    {
      delete *it;
      it = m_scenes.erase(it);
    }
    else
    {
      ++it;
    }
  }
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

  for (auto it = m_pictures.begin(); it != m_pictures.end(); ++it)
  {
    it->second->draw(m_targetTexture);
  }

  if (m_flashCounter > 0)
  {
    float percent = (float)m_flashCounter / (float)m_flashDuration;
    float opacity = 255 - 255 * percent;

    sf::Color fillColor = m_flashColor;
    fillColor.a = opacity;

    sf::RectangleShape flashRect;
    flashRect.setSize(sf::Vector2f(m_targetTexture.getSize().x, m_targetTexture.getSize().y));
    flashRect.setFillColor(fillColor);
    m_targetTexture.draw(flashRect);
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

void SceneManager::flashScreen(int duration, sf::Color color)
{
  m_flashCounter = duration;
  m_flashDuration = duration;
  m_flashColor = color;
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

void SceneManager::showPicture(const std::string& pictureName, float x, float y)
{
  auto it = m_pictures.find(pictureName);

  if (it == m_pictures.end())
  {
    Picture* picture = new Picture(pictureName);
    picture->setPosition(x, y);
    m_pictures[pictureName] = picture;
  }
  else
  {
    it->second->setPosition(x, y);
  }
}

void SceneManager::hidePicture(const std::string& pictureName)
{
  auto it = m_pictures.find(pictureName);
  if (it != m_pictures.end())
  {
    delete it->second;
    m_pictures.erase(it);
  }
}

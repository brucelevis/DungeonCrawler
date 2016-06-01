#include <cmath>

#include "Cache.h"
#include "Sound.h"
#include "Persistent.h"
#include "SceneManager.h"
#include "Player.h"
#include "PlayerCharacter.h"
#include "Config.h"
#include "Message.h"
#include "draw_text.h"
#include "Frame.h"
#include "Vocabulary.h"

#include "Campsite.h"

Campsite::Campsite()
 : m_state(STATE_STARTING),
   m_requiredFood(requiredFood()),
   m_background( cache::loadTexture("Backgrounds/Campsite.png") ),
   m_draw(true)
{
  m_menu.addEntry("Rest");
  m_menu.addEntry("Leave");
  m_menu.setVisible(false);

  m_restMusic.openFromFile( config::res_path("Music/Inn.ogg") );

  SceneManager::instance().fadeIn(32);
}

Campsite::~Campsite()
{
  cache::releaseTexture(m_background);
}

void Campsite::update()
{
  if (m_state == STATE_START_MESSAGE)
  {
    Message::instance().show(m_currentMessage);
    m_state = STATE_MESSAGE_ONGOING;
  }
  else if (m_state == STATE_MESSAGE_ONGOING)
  {
    Message::instance().update();

    if (Message::instance().isWaitingForKey())
    {
      m_state = STATE_CHOOSE;
      m_menu.setVisible(true);
    }
  }
  else if (m_state == STATE_REST)
  {
    if (m_restMusic.getStatus() == sf::Music::Stopped)
    {
      leave();
    }
  }
}

void Campsite::draw(sf::RenderTexture& target)
{
  if (m_draw)
  {
    sf::Sprite sprite;
    sprite.setTexture(*m_background);
    target.draw(sprite);

    if (m_state != STATE_REST && m_state != STATE_STARTING && m_state != STATE_LEAVE)
    {
      Message::instance().draw(target);

      if (m_menu.isVisible())
      {
        m_menu.draw(target, 0, config::GAME_RES_Y - 68 - m_menu.getHeight());
      }

      draw_frame(target, 0, 0, 128, 24);
      draw_text_bmp(target, 8, 8, "Food: %d / %d", m_requiredFood, global<int>("$sys:food"));
    }
  }
}

void Campsite::handleEvent(sf::Event& event)
{
  if (event.type == sf::Event::KeyPressed)
  {
    auto key = event.key.code;

    if (key == sf::Keyboard::Space)
    {
      if (m_state == STATE_MESSAGE_ONGOING)
      {
        if (!Message::instance().isWaitingForKey())
        {
          Message::instance().flush();
        }
      }
      else if (m_state == STATE_CHOOSE)
      {
        if (m_menu.getCurrentMenuChoice() == "Rest")
        {
          if (global<int>("$sys:food") >= m_requiredFood)
          {
            rest();
            set_global("$sys:food", global<int>("$sys:food") - m_requiredFood);
          }
          else
          {
            startMessage("You don't have enough food to rest right now.");
            m_menu.setVisible(false);

            play_sound(config::get("SOUND_CANCEL"));
          }
        }
        else if (m_menu.getCurrentMenuChoice() == "Leave")
        {
          leave();
        }
      }
    }
    else if (key == sf::Keyboard::Up)
    {
      if (m_state == STATE_CHOOSE)
      {
        m_menu.moveArrow(DIR_UP);
      }
    }
    else if (key == sf::Keyboard::Down)
    {
      if (m_state == STATE_CHOOSE)
      {
        m_menu.moveArrow(DIR_DOWN);
      }
    }
    else if (key == sf::Keyboard::Escape)
    {
      if (m_state == STATE_CHOOSE)
      {
        leave();
      }
    }
  }
}

void Campsite::preFade(Scene::FadeType fadeType)
{

}

void Campsite::postFade(Scene::FadeType fadeType)
{
  if (fadeType == Scene::FADE_OUT)
  {
    if (m_state == STATE_LEAVE)
    {
      SceneManager::instance().fadeIn(32);
      close();
    }
    else if (m_state == STATE_REST)
    {
      m_draw = false;
    }
  }
  else if (fadeType == Scene::FADE_IN)
  {
    if (m_state == STATE_STARTING)
    {
      startMessage("You feel the warm glow of the camp site surround you. It is safe to rest here.");
    }
  }
}

void Campsite::rest()
{
  get_player()->recoverAll();

  m_state = STATE_REST;
  m_restMusic.play();
  SceneManager::instance().fadeOut(32);
}

void Campsite::leave()
{
  Message::instance().clear();

  m_state = STATE_LEAVE;
  SceneManager::instance().fadeOut(32);
}

int Campsite::requiredFood() const
{
  float sumLevel = 0;
  float multiplier = 1;

  for (auto pc : get_player()->getParty())
  {
    sumLevel += pc->computeCurrentAttribute(terms::level);

    if (pc->hasStatus("Dead"))
    {
      multiplier += 0.5f;
    }
    else
    {
      int maxHp = pc->getAttribute(terms::hp).max;
      int curHp = pc->getAttribute(terms::hp).current;

      float quote = (float) curHp / (float) maxHp;

      if (quote < 0.25f)      multiplier += 0.15f;
      else if (quote < 0.5f)  multiplier += 0.1f;
      else if (quote < 0.75f) multiplier += 0.05f;
    }
  }

  return (int)ceilf(sumLevel * multiplier);
}

void Campsite::startMessage(const std::string& message)
{
  Message::instance().clear();
  m_currentMessage = message;
  m_state = STATE_START_MESSAGE;
}

#include "BattleAnimation.h"
#include "Config.h"
#include "Game.h"
#include "Flash.h"

Flash::Flash()
 : m_ticks(0),
   m_speed(0),
   m_numberOfFlashes(0),
   m_currentFlash(0),
   m_activeAnimation(0),
   m_fadeSpeed(0),
   m_fadeCounter(0),
   m_shakeCounter(0),
   m_shakePower(0)
{

}

Flash::~Flash()
{
  delete m_activeAnimation;
}

void Flash::start(int number, int speed)
{
  m_ticks = 0;
  m_speed = speed;
  m_numberOfFlashes = number;
  m_currentFlash = 0;
}

void Flash::startEffect(const std::string& effectName)
{
  if (!m_activeAnimation && !effectName.empty())
  {
    m_activeAnimation = BattleAnimation::loadBattleAnimation(config::res_path("Animations/" + effectName));
  }
}

void Flash::update()
{
  for (auto it = m_damageNumbers.begin(); it != m_damageNumbers.end();)
  {
    it->color.a = 255 - (2 * it->life);
    it->life++;
    if (it->life == 32)
    {
      it = m_damageNumbers.erase(it);
    }
    else
    {
      ++it;
    }
  }

  if (isFlashing())
  {
    m_ticks++;
    if (m_ticks >= m_speed)
    {
      m_currentFlash++;
      m_ticks = 0;
    }
  }

  if (isFading())
  {
    m_fadeCounter -= m_fadeSpeed;
  }

  if (activeBattleAnimation())
  {
    if (!m_activeAnimation->complete())
    {
      m_activeAnimation->update();
    }
    else
    {
      delete m_activeAnimation;
      m_activeAnimation = 0;
    }
  }

  if (isShaking())
  {
    m_shakeCounter--;
  }
}

void Flash::fadeOut(int speed)
{
  m_fadeCounter = 255;
  m_fadeSpeed = speed;
}

void Flash::addDamageText(const std::string& text, const sf::Color& color)
{
  // Only add damage text if we are in combat.
  if (!Game::instance().battleInProgress())
  {
    return;
  }

  DamageText dmgText { text, 0, color };
  if (m_damageNumbers.size())
  {
    if (m_damageNumbers.back().life == 0)
    {
      for (auto it = m_damageNumbers.begin(); it != m_damageNumbers.end(); ++it)
      {
        it->life += 8;
      }
    }
  }
  m_damageNumbers.push_back(dmgText);
}

void Flash::shake(int duration, int power)
{
  m_shakeCounter = duration;
  m_shakePower = power;
}

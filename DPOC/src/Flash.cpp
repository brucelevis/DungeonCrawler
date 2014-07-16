#include "Flash.h"

Flash::Flash()
 : m_ticks(0),
   m_speed(0),
   m_numberOfFlashes(0),
   m_currentFlash(0),
   m_activeEffect(0),
   m_fadeSpeed(0),
   m_fadeCounter(0)
{

}

Flash::~Flash()
{
  delete m_activeEffect;
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
  if (!m_activeEffect && !effectName.empty())
  {
    m_activeEffect = Effect::createEffect(effectName);
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

  if (activeEffect())
  {
    if (!m_activeEffect->complete())
    {
      m_activeEffect->update();
    }
    else
    {
      delete m_activeEffect;
      m_activeEffect = 0;
    }
  }
}

void Flash::fadeOut(int speed)
{
  m_fadeCounter = 255;
  m_fadeSpeed = speed;
}

void Flash::addDamageText(const std::string& text, const sf::Color& color)
{
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

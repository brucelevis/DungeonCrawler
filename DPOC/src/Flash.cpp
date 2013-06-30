#include "Flash.h"

Flash::Flash()
 : m_ticks(0),
   m_speed(0),
   m_numberOfFlashes(0),
   m_currentFlash(0)
{

}

void Flash::start(int number, int speed)
{
  m_ticks = 0;
  m_speed = speed;
  m_numberOfFlashes = number;
  m_currentFlash = 0;
}

void Flash::update()
{
  if (!isFlashing())
    return;

  m_ticks++;
  if (m_ticks >= m_speed)
  {
    m_currentFlash++;
    m_ticks = 0;
  }
}

#ifndef FLASH_H
#define FLASH_H

class Flash
{
public:
  void start(int number, int speed);

  bool isFlashing() const { return m_currentFlash < m_numberOfFlashes; }

  void update();

  bool odd() const { return (m_currentFlash % 2) == 1; }
  bool even() const { return (m_currentFlash % 2) == 0; }
private:
  int m_ticks, m_speed;
  int m_numberOfFlashes, m_currentFlash;
};

#endif

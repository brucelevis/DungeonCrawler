#ifndef FLASH_H
#define FLASH_H

class Flash
{
public:
  Flash(int number, int speed);

  bool isDone() const { return m_numberOfFlashes >= m_currentFlash; }

  void update();

  bool odd() const { return (m_currentFlash % 2) == 1; }
  bool event() const { return (m_currentFlash % 2) == 0; }
private:
  int m_ticks, m_speed;
  int m_numberOfFlashes, m_currentFlash;
};

#endif

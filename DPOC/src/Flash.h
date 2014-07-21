#ifndef FLASH_H
#define FLASH_H

#include <string>
#include <vector>

#include "Effect.h"

struct DamageText
{
  std::string text;
  int life;
  sf::Color color;
};

class Flash
{
public:
  Flash();
  ~Flash();

  void start(int number, int speed);
  void startEffect(const std::string& effectName);

  bool isFlashing() const { return m_currentFlash < m_numberOfFlashes; }
  bool isFading() const { return m_fadeCounter > 0; }

  void update();

  bool odd() const { return (m_currentFlash % 2) == 1; }
  bool even() const { return (m_currentFlash % 2) == 0; }

  BattleAnimation *activeEffect() const { return m_activeEffect; }

  void fadeOut(int speed);
  int fadeCounter() const { return m_fadeCounter; }

  void addDamageText(const std::string& text, const sf::Color& color);
  const std::vector<DamageText>& damageText() const { return m_damageNumbers; }
private:
  Flash(const Flash&);
  Flash& operator=(const Flash&);
private:
  int m_ticks, m_speed;
  int m_numberOfFlashes, m_currentFlash;

  BattleAnimation* m_activeEffect;

  int m_fadeSpeed;
  int m_fadeCounter;

  std::vector<DamageText> m_damageNumbers;
};

#endif

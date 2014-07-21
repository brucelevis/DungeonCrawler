#ifndef EFFECT_H_
#define EFFECT_H_

#include <string>

class Character;

struct Effect
{
  std::string animation;
  std::string sound;

  void playSfx() const;
  void applyAnimation(Character* character) const;
};

#endif

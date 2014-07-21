#ifndef EFFECT_H_
#define EFFECT_H_

#include <string>

namespace tinyxml2
{
  class XMLElement;
}

class Character;

struct Effect
{
  std::string animation;
  std::string sound;

  void playSfx() const;
  void applyAnimation(Character* character) const;

  static Effect createFromXmlElement(const tinyxml2::XMLElement* element);
};

#endif

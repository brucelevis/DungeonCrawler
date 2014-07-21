#include "Sound.h"
#include "Character.h"
#include "Flash.h"

#include "../dep/tinyxml2.h"
using namespace tinyxml2;

void Effect::playSfx() const
{
  if (sound.size())
  {
    play_sound("Audio/" + sound);
  }
}

void Effect::applyAnimation(Character* character) const
{
  if (animation.size())
  {
    character->flash().startEffect(animation);
  }
}

Effect Effect::createFromXmlElement(const tinyxml2::XMLElement* element)
{
  Effect effect;

  const XMLAttribute* animAttr = element->FindAttribute("animation");
  const XMLAttribute* sndAttr  = element->FindAttribute("sound");

  if (animAttr)
    effect.animation = animAttr->Value();
  if (sndAttr)
    effect.sound = sndAttr->Value();

  return effect;
}

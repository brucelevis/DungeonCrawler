#include "Sound.h"
#include "Character.h"
#include "Flash.h"

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

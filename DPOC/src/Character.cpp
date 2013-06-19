#include "Cache.h"
#include "Character.h"

Character::~Character()
{
  cache::releaseTexture("Resources/Faces/Face.png");
}

Character* Character::create(const std::string& name)
{
  Character* character = new Character;

  character->m_name = name;
  character->m_spells.push_back("Hurt");
  character->m_spells.push_back("Heal");
  character->m_faceTexture = cache::loadTexture("Resources/Faces/Face.png");

  return character;
}

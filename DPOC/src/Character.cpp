#include "Cache.h"
#include "Character.h"

Character::~Character()
{
  cache::releaseTexture("Resources/Faces/Face.png");
}

Attribute& Character::getAttribute(const std::string& attribName)
{
  auto it = m_attributes.find(attribName);
  if (it != m_attributes.end())
  {
    return it->second;
  }

  throw std::runtime_error("Attribute " + attribName + " does not exist on character " + getName());
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

#include "logger.h"
#include "Cache.h"
#include "Utility.h"
#include "Monster.h"
#include "Character.h"

Character::~Character()
{
  cache::releaseTexture(m_faceTexture);
}

Attribute& Character::getAttribute(const std::string& attribName)
{
  std::string lowerCase = to_lower(attribName);

  auto it = m_attributes.find(lowerCase);
  if (it != m_attributes.end())
  {
    return it->second;
  }

  TRACE("Attribute %s does not exist on character %s", lowerCase.c_str(), getName().c_str());

  throw std::runtime_error("Attribute " + lowerCase + " does not exist on character " + getName());
}

int Character::computeCurrentAttribute(const std::string& attribName)
{
  int sum = getAttribute(attribName).current;

  return sum;
}

void Character::draw(sf::RenderTarget& target, int x, int y) const
{
  bool shouldDraw = true;

  if (m_flash.isFlashing())
  {
    shouldDraw = m_flash.odd();
  }

  if (shouldDraw)
  {
    sf::Sprite sprite;
    sprite.setTexture(*m_faceTexture);
    sprite.setTextureRect(m_textureRect);
    sprite.setPosition(x, y);
    target.draw(sprite);
  }

  if (m_flash.activeEffect())
  {
    int posX = x + spriteWidth() / 2 - m_flash.activeEffect()->spriteSize() / 2;
    int posY = y + spriteHeight() / 2 - m_flash.activeEffect()->spriteSize() / 2;

    m_flash.activeEffect()->setOrigin(posX, posY);
    m_flash.activeEffect()->render(target);
  }
}

bool Character::incapacitated() const
{
  return getStatus() == "Dead" ||
         getStatus() == "Paralyze" ||
         getStatus() == "Sleep";
}

void Character::resetStatus()
{
  setStatus("Normal");
}

void Character::takeDamage(const std::string& attr, int amount)
{
  getAttribute(attr).current -= amount;
  clamp_attribute(getAttribute(attr));
}

Character* Character::createMonster(const std::string& name)
{
  MonsterDef def = get_monster_definition(name);

  if (def.name == BUG_MONSTER)
    return 0;

  Character* character = new Character;

  character->m_name = def.name;
  character->m_faceTexture = cache::loadTexture(def.texture);
  character->m_textureRect = def.textureRect;
  character->m_status = "Normal";

  for (auto it = def.attributeMap.begin(); it != def.attributeMap.end(); ++it)
  {
    character->m_attributes[it->first] = make_attribute(it->second);
  }

  return character;
}

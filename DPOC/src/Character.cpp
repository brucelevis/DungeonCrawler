#include "logger.h"
#include "Cache.h"
#include "Utility.h"
#include "Monster.h"
#include "StatusEffect.h"
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

bool Character::afflictStatus(const std::string& status)
{
  auto it = getStatusEffectIterator(status);

  if (it == m_status.end())
  {
    // Dead status removes all other statuses.
    if (status == "Dead")
    {
      resetStatus();
    }

    if (hasStatus("Normal"))
    {
      m_status.clear();
    }

    m_status.push_back(get_status_effect(status));

    return true;
  }

  return false;
}

bool Character::cureStatus(const std::string& status)
{
  auto it = getStatusEffectIterator(status);

  if (it != m_status.end())
  {
    m_status.erase(it);

    if (m_status.empty())
      resetStatus();

    return true;
  }

  return false;
}

bool Character::hasStatus(const std::string& status)
{
  return getStatusEffectIterator(status) != m_status.end();
}

std::string Character::getStatus() const
{
  // Always display most recent status.
  if (m_status.size() > 0)
    return m_status.back()->name;

  return "Normal";
}

bool Character::incapacitated() const
{
  for (auto it = m_status.begin(); it != m_status.end(); ++it)
  {
    if ((*it)->incapacitate)
      return true;
  }

  return false;
}

void Character::resetStatus()
{
  m_status.clear();
  m_status.push_back(get_status_effect("Normal"));
}

std::vector<StatusEffect*>::iterator Character::getStatusEffectIterator(const std::string& status)
{
  for (auto it = m_status.begin(); it != m_status.end(); ++it)
  {
    if (to_lower((*it)->name) == to_lower(status))
      return it;
  }
  return m_status.end();
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
  character->m_status.push_back(get_status_effect("Normal"));

  for (auto it = def.attributeMap.begin(); it != def.attributeMap.end(); ++it)
  {
    character->m_attributes[it->first] = make_attribute(it->second);
  }

  return character;
}

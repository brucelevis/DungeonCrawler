#include "logger.h"
#include "Cache.h"
#include "Utility.h"
#include "Monster.h"
#include "Character.h"

Attribute make_attribute(int val)
{
  return { val, val };
}

std::vector<std::string> Character::equipNames =
{
  "Weapon", "Shield", "Armour", "Helmet", "Others", "Others"
};

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

  for (auto it = m_equipment.begin(); it != m_equipment.end(); ++it)
  {
    auto gainIt = it->second.attributeGain.find(attribName);
    if (gainIt != it->second.attributeGain.end())
    {
      sum += gainIt->second;
    }
  }

  return sum;
}

void Character::equip(const std::string& equipmentSlot, const std::string& itemName)
{
  if (itemName.empty())
  {
    m_equipment.erase(to_lower(equipmentSlot));
  }
  else
  {
    m_equipment[to_lower(equipmentSlot)] = create_item(itemName, 1);
  }
}

Item* Character::getEquipment(const std::string& equipmentSlot)
{
  auto it = m_equipment.find(to_lower(equipmentSlot));

  if (it != m_equipment.end())
  {
    return &it->second;
  }

  return 0;
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
  return getStatus() == "Dead";
}


int Character::toNextLevel()
{
  return expForLevel() - getAttribute("exp").max;
}

int Character::expForLevel()
{
  int level = getAttribute("level").max;

  return level * level * 10;
}

int Character::checkLevelUp()
{
  int levelReached = 0;
  int exp = getAttribute("exp").max;

  while (exp > expForLevel())
  {
    getAttribute("level").max++;
    clamp_attribute(getAttribute("level"));
    levelReached = getAttribute("level").max;
  }

  return levelReached;
}

Character* Character::create(const std::string& name)
{
  Character* character = new Character;

  character->m_name = name;
  character->m_spells.push_back("Hurt");
  character->m_spells.push_back("Heal");
  character->m_spells.push_back("Hurt All");
  character->m_faceTexture = cache::loadTexture("Resources/Faces/Face.png");
  character->m_textureRect = sf::IntRect(0, 0, character->m_faceTexture->getSize().x, character->m_faceTexture->getSize().y);

  character->m_status = "Normal";

  character->m_attributes["hp"] = make_attribute(30);
  character->m_attributes["mp"] = make_attribute(15);

  character->m_attributes["level"] = make_attribute(1);
  character->m_attributes["exp"] = make_attribute(0);

  character->m_attributes["strength"] = make_attribute(20);
  character->m_attributes["power"] = make_attribute(15);
  character->m_attributes["defense"] = make_attribute(15);
  character->m_attributes["magic"] = make_attribute(10);
  character->m_attributes["mag.def"] = make_attribute(10);
  character->m_attributes["speed"] = make_attribute(15);

  // character->m_equipment["weapon"] = create_item("Rusty Knife");
  // character->m_equipment["shield"] = create_item("Wood Shield");

  return character;
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

#include "Utility.h"
#include "Cache.h"
#include "PlayerCharacter.h"

std::vector<std::string> PlayerCharacter::equipNames =
{
  "Weapon", "Shield", "Armour", "Helmet", "Others", "Others"
};

void PlayerCharacter::equip(const std::string& equipmentSlot, const std::string& itemName)
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

Item* PlayerCharacter::getEquipment(const std::string& equipmentSlot)
{
  auto it = m_equipment.find(to_lower(equipmentSlot));

  if (it != m_equipment.end())
  {
    return &it->second;
  }

  return 0;
}

int PlayerCharacter::computeCurrentAttribute(const std::string& attribName)
{
  int sum = Character::computeCurrentAttribute(attribName);

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

int PlayerCharacter::toNextLevel()
{
  return expForLevel() - getAttribute("exp").max;
}

int PlayerCharacter::expForLevel()
{
  int level = getAttribute("level").max;

  return level * level * 10;
}

int PlayerCharacter::checkLevelUp()
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

PlayerCharacter* PlayerCharacter::create(const std::string& name)
{
  PlayerCharacter* character = new PlayerCharacter;

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

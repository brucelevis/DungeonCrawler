#include "Utility.h"
#include "Cache.h"
#include "Message.h"

#include "PlayerCharacter.h"

std::vector<std::string> PlayerCharacter::equipNames =
{
  "Weapon", "Shield", "Armour", "Helmet", "Others", "Others"
};

PlayerCharacter::PlayerCharacter()
 : m_class(player_class_ref("Hero"))
{
}

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

bool PlayerCharacter::canEquip(const std::string& itemName)
{
  return std::find(m_class.equipment.begin(), m_class.equipment.end(), itemName) != m_class.equipment.end();
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

int PlayerCharacter::checkLevelUp(bool display)
{
  int levelReached = 0;
  int exp = getAttribute("exp").max;

  while (exp > expForLevel())
  {
    getAttribute("level").max++;
    reset_attribute(getAttribute("level"));
    levelReached = getAttribute("level").max;
  }

  setLevel(levelReached, display);

  return levelReached;
}

void PlayerCharacter::setAttributes()
{
  int level = m_attributes["level"].max;

  for (auto it = m_class.baseAttributes.begin(); it != m_class.baseAttributes.end(); ++it)
  {
    static const int EV = 128;
    static const int IV = 16;

    int base = it->second;
    int attrib;

    if (it->first == "hp" || it->first == "mp")
    {
      attrib = 10 + ((IV + (2 * base) + (EV / 4) + 100) * level) / 100;

      m_attributes[it->first].max = attrib;
    }
    else
    {
      attrib = 5 + ((IV + (2 * base) + (EV / 4)) * level) / 100;

      m_attributes[it->first] = make_attribute(attrib);
    }
  }
}

void PlayerCharacter::setLevel(int levelReached, bool display)
{
  m_attributes["level"] = make_attribute(levelReached);

  std::map<std::string, Attribute> attributesTemp = m_attributes;

  setAttributes();

  if (levelReached > 0)
  {
    if (display)
      show_message("%s has reached level %d!", getName().c_str(), levelReached);

    std::string buffer;

    for (auto it = m_attributes.begin(); it != m_attributes.end(); ++it)
    {
      int increase = it->second.max - attributesTemp[it->first].max;

      if (it->first != "level" && it->first != "exp")
        buffer += capitalize(it->first) + " +" + toString(increase) + "! ";
    }

    if (display)
      show_message("%s", buffer.c_str());

    if (m_class.spells.count(levelReached) > 0)
    {
      auto it = m_class.spells.find(levelReached);

      for (auto spellIt = it->second.begin(); spellIt != it->second.end(); ++spellIt)
      {
        if (std::find(m_spells.begin(), m_spells.end(), *spellIt) == m_spells.end())
        {
          m_spells.push_back(*spellIt);
          if (display)
            show_message("%s learned %s!", getName().c_str(), spellIt->c_str());
        }
      }
    }
  }
}

PlayerCharacter* PlayerCharacter::create(const std::string& name)
{
  PlayerCharacter* character = new PlayerCharacter;

  character->m_name = name;
  character->m_faceTexture = cache::loadTexture("Resources/Faces/Face.png");
  character->m_textureRect = sf::IntRect(0, 0, character->m_faceTexture->getSize().x, character->m_faceTexture->getSize().y);

  character->m_class = player_class_ref("Hero");

  character->m_status = "Normal";

  character->m_attributes["level"] = make_attribute(0);
  character->m_attributes["exp"] = make_attribute(0);

  character->setLevel(1, false);
  reset_attribute(character->m_attributes["hp"]);
  reset_attribute(character->m_attributes["mp"]);

  // character->m_equipment["weapon"] = create_item("Rusty Knife");
  // character->m_equipment["shield"] = create_item("Wood Shield");

  return character;
}

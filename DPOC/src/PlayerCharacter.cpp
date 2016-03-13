#include <sstream>

#include "SaveLoad.h"

#include "BattleAnimation.h"
#include "Config.h"
#include "Utility.h"
#include "Cache.h"
#include "Message.h"
#include "StatusEffect.h"
#include "Skill.h"
#include "Vocabulary.h"

#include "PlayerCharacter.h"

PlayerCharacter::PlayerCharacter()
 : m_skullTexture(cache::loadTexture("Pictures/Death.png"))
{
}

PlayerCharacter::~PlayerCharacter()
{
  cache::releaseTexture(m_skullTexture);
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

bool PlayerCharacter::canEquip(const Item& item) const
{
  return (std::find(m_class.equipment.begin(), m_class.equipment.end(), item.name) != m_class.equipment.end()) &&
          meetsPrereqsForItem(item);
}

bool PlayerCharacter::meetsPrereqsForItem(const Item& item) const
{
  // If any attribute in the prereqs list is less than the required,
  // return false.

  for (const auto& pair : item.prerequisites)
  {
    int attrValue = getBaseAttribute(pair.first);

    if (attrValue < pair.second)
    {
      return false;
    }
  }

  return true;
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

int PlayerCharacter::getBaseAttribute(const std::string& attribName) const
{
  auto it = m_attributes.find(attribName);

  if (it != m_attributes.end())
  {
    return it->second.max;
  }

  return 0;
}

void PlayerCharacter::advanceAttribute(const std::string& attribName, int value)
{
  auto it = m_attributes.find(attribName);

  if (it == m_attributes.end())
  {
    m_attributes[attribName].current = value;
    m_attributes[attribName].max = value;
  }
  else
  {
    it->second.current += value;
    it->second.max += value;
  }
}

int PlayerCharacter::toNextLevel()
{
  return expForLevel() - getAttribute(terms::exp).max;
}

int PlayerCharacter::expForLevel()
{
  int level = getAttribute(terms::level).max;

//  return level * level * 10;

  // TODO: Put in XML.
  static const int base = 33;
  static const int modifier = 38;

  return base * level / 4 + modifier * level * level * level / 8;
}

int PlayerCharacter::checkLevelUp(bool display)
{
  int levelReached = 0;
  int exp = getAttribute(terms::exp).max;

  while (exp > expForLevel())
  {
    getAttribute(terms::level).max++;
    reset_attribute(getAttribute(terms::level));
    levelReached = getAttribute(terms::level).max;
  }

  if (levelReached > 0)
  {
    setLevel(levelReached, display);
  }

  return levelReached;
}

void PlayerCharacter::setAttributes()
{
  int level = m_attributes[terms::level].max;

  for (auto it = m_class.baseAttributes.begin(); it != m_class.baseAttributes.end(); ++it)
  {
    int base = it->second.base;
    int max  = it->second.max;

    if (base == 0)
      continue;

    if (level > 1)
    {
      float base_percent = (float)level / fromString<float>(config::get("MAX_LEVEL"));
      int   base_attrib  = base + base_percent * (float)max;

      float prev_percent = (float)(level - 1) / fromString<float>(config::get("MAX_LEVEL"));
      int   prev_attrib   = base + prev_percent * (float)max;

      int delta = base_attrib - prev_attrib;

      m_attributes[it->first].max += delta;
    }
    else
    {
      float percent = (float)level / fromString<float>(config::get("MAX_LEVEL"));
      int attrib = base + percent * (float)max;

      m_attributes[it->first].max = attrib;
    }

    // HP/MP is not restores when leveling up.
    if (it->first != terms::hp && it->first != terms::mp)
    {
      reset_attribute(m_attributes[it->first]);
    }
  }

  if (level > 1)
  {
    for (const auto& fixedAttributePair : m_class.fixedAttributes)
    {
      advanceAttribute(fixedAttributePair.first, fixedAttributePair.second);
    }
  }
}

void PlayerCharacter::setLevel(int levelReached, bool display)
{
  m_attributes[terms::level] = make_attribute(levelReached);

  std::map<std::string, Attribute> attributesTemp = m_attributes;

  setAttributes();

  if (levelReached > 0)
  {
    if (display)
    {
      show_message("%s has reached level %d!", getName().c_str(), levelReached);
    }

    std::string buffer;

    for (auto it = m_attributes.begin(); it != m_attributes.end(); ++it)
    {
      // Don't increase skills on levelup.
      if (Skill::isSkill(it->first))
        continue;

      int increase = it->second.max - attributesTemp[it->first].max;

      if (it->first != terms::level && it->first != terms::exp)
        buffer += vocab(it->first) + " +" + toString(increase) + "! ";
    }

    if (display)
    {
      show_message("%s", buffer.c_str());
    }

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

std::string PlayerCharacter::xmlDump() const
{
  std::ostringstream xml;
  xml << "<playerCharacter name=\"" << getName() << "\">\n";

  xml << " <equipment>\n";
  for (auto it = m_equipment.begin(); it != m_equipment.end(); ++it)
  {
    xml << "<" << it->first << ">" << it->second.name << "</" << it->first << ">\n";
  }
  xml << " </equipment>\n";

  xml << " <spells>\n";
  for (auto it = m_spells.begin(); it != m_spells.end(); ++it)
  {
    xml << "<spell>" << (*it) << "</spell>\n";
  }
  xml << " </spells>\n";

  xml << " <class>" << m_class.name << "</class>\n";

  xml << " <texture name=\""
      << cache::getTextureName(m_faceTexture)
      << "\" x=\""
      << m_textureRect.left
      << "\" y=\""
      << m_textureRect.top
      << "\" w=\""
      << m_textureRect.width
      << "\" h=\""
      << m_textureRect.height
      << "\" />\n";

  xml << " <attributes>\n";
  for (auto it = m_attributes.begin(); it != m_attributes.end(); ++it)
  {
    xml << "  <attribute name=\"" << it->first
        << "\" current=\"" << it->second.current
        << "\" max=\"" << it->second.max << "\" />\n";
  }
  xml << " </attributes>\n";

  xml << " <statusEffects>\n";
  for (auto it = m_status.begin(); it != m_status.end(); ++it)
  {
    xml << "  <status>" << (*it)->name << "</status>\n";
  }
  xml << " </statusEffects>\n";

  xml << "</playerCharacter>\n";

  return xml.str();
}

float PlayerCharacter::getResistance(const std::string& element) const
{
  float resistSum = 1.0f;

  for (auto it = m_equipment.begin(); it != m_equipment.end(); ++it)
  {
    auto elemIt = it->second.elements.find(element);
    if (elemIt != it->second.elements.end())
    {
      resistSum *= elemIt->second;
    }
  }

  return Character::getResistance(element);
}

bool PlayerCharacter::isImmune(const std::string& status) const
{
  for (auto it = m_equipment.begin(); it != m_equipment.end(); ++it)
  {
    if (std::find(it->second.status.begin(), it->second.status.end(), status) != it->second.status.end())
    {
      return true;
    }
  }

  return Character::isImmune(status);
}

void PlayerCharacter::setClass(const std::string& className)
{
  m_class = player_class_ref(className);
  setUnarmedAttackEffect(m_class.unarmedAttackEffect);
}

void PlayerCharacter::draw(sf::RenderTarget& target, int x, int y) const
{
  bool isDead = false;

  for (auto it = m_status.begin(); it != m_status.end(); ++it)
  {
    if (to_lower((*it)->name) == "dead")
    {
      isDead = true;
      break;
    }
  }

  if (isDead && !flash().isFading())
  {
    sf::Sprite sprite;
    sprite.setTexture(*m_skullTexture);
    sprite.setTextureRect(sf::IntRect(0, 0, m_skullTexture->getSize().x, m_skullTexture->getSize().y));
    sprite.setPosition(x, y);
    target.draw(sprite);

    if (m_flash.activeBattleAnimation())
    {
      int posX = x + m_skullTexture->getSize().x / 2;
      int posY = y + m_skullTexture->getSize().y / 2;

      m_flash.activeBattleAnimation()->setOrigin(posX, posY);
      m_flash.activeBattleAnimation()->render(target);
    }
  }
  else
  {
    if (isDead)
    {
      sf::Sprite sprite;
      sprite.setTexture(*m_skullTexture);
      sprite.setTextureRect(sf::IntRect(0, 0, m_skullTexture->getSize().x, m_skullTexture->getSize().y));
      sprite.setPosition(x, y);
      sprite.setColor(sf::Color(255, 255, 255, 255 - flash().fadeCounter()));
      target.draw(sprite);
    }

    Character::draw(target, x, y);
  }
}

PlayerCharacter* PlayerCharacter::create(const std::string& name, const std::string& className, int level)
{
  PlayerCharacter* character = create(name, className, player_class_ref(className).faceTexture, level);
  character->m_textureRect = character->m_class.textureRect;
  return character;
}

PlayerCharacter* PlayerCharacter::create(const std::string& name, const std::string& className, const std::string& face, int level)
{
  PlayerCharacter* character = new PlayerCharacter;

  character->m_name = name;
  character->setClass(className);

  character->m_faceTexture = cache::loadTexture(face);
  character->m_textureRect = sf::IntRect(0, 0, character->m_faceTexture->getSize().x, character->m_faceTexture->getSize().y);

  character->m_status.push_back(get_status_effect("Normal"));

  character->m_attributes[terms::level] = make_attribute(0);
  character->m_attributes[terms::exp] = make_attribute(0);
  character->m_attributes[terms::skillpoints] = make_attribute(0);

  for (int i = 1; i <= level; i++)
  {
    character->setLevel(i, false);

    if (i < level)
    {
      // Gain enough exp for the current level.
      character->getAttribute(terms::exp).max = character->expForLevel();
      reset_attribute(character->getAttribute(terms::exp));
    }
  }

  reset_attribute(character->m_attributes[terms::hp]);
  reset_attribute(character->m_attributes[terms::mp]);

  return character;
}

PlayerCharacter* PlayerCharacter::createFromSaveData(CharacterData* data)
{
  PlayerCharacter* character = new PlayerCharacter;

  character->m_name = data->name;
  character->m_faceTexture = cache::loadTexture(data->textureName);
  character->m_textureRect = sf::IntRect(data->textureX, data->textureY, data->textureW, data->textureH);

  character->setClass(data->className);

  for (auto it = data->statusEffects.begin(); it != data->statusEffects.end(); ++it)
  {
    character->m_status.push_back(get_status_effect(*it));
  }

  for (auto it = data->attributes.begin(); it != data->attributes.end(); ++it)
  {
    character->m_attributes[it->first] = it->second;
  }

  for (auto it = data->spells.begin(); it != data->spells.end(); ++it)
  {
    character->m_spells.push_back(*it);
  }

  for (auto it = data->equipment.begin(); it != data->equipment.end(); ++it)
  {
    character->m_equipment[it->first] = create_item(it->second, 1);
  }

  return character;
}

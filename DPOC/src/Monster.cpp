#include <vector>
#include <stdexcept>

#include "Config.h"
#include "Utility.h"
#include "logger.h"
#include "Monster.h"

#include "../dep/tinyxml2.h"

using namespace tinyxml2;

static std::vector<MonsterDef> monsters;

static MonsterDef parse_monster_element(const XMLElement* monsterElement)
{
  MonsterDef monster;
  // Monsters does not care about MP but it's required for spellcasting.
  monster.attributeMap["mp"] = 0;
  monster.color = sf::Color::White;
  monster.numberOfAttacks = 1;

  const XMLElement* nameElem = monsterElement->FirstChildElement("name");
  const XMLElement* descElem = monsterElement->FirstChildElement("description");
  const XMLElement* textElem = monsterElement->FirstChildElement("texture");
  const XMLElement* colorElem = monsterElement->FirstChildElement("color");
  const XMLElement* numAttacksElem = monsterElement->FirstChildElement("numberOfAttacks");
  const XMLElement* effeElem = monsterElement->FirstChildElement("attackEffect");

  if (nameElem)
    monster.name = nameElem->GetText();
  if (descElem)
    monster.description = descElem->GetText();
  if (colorElem)
  {
    int r = fromString<int>(colorElem->FindAttribute("r")->Value());
    int g = fromString<int>(colorElem->FindAttribute("g")->Value());
    int b = fromString<int>(colorElem->FindAttribute("b")->Value());
    monster.color = sf::Color(r, g, b);
  }
  if (numAttacksElem)
  {
    monster.numberOfAttacks = fromString<int>(numAttacksElem->GetText());
  }
  if (effeElem)
  {
    monster.attackEffect = Effect::createFromXmlElement(effeElem);
  }

  if (textElem)
  {
    const XMLAttribute* nameAttr = textElem->FindAttribute("name");
    const XMLAttribute* xAttr = textElem->FindAttribute("x");
    const XMLAttribute* yAttr = textElem->FindAttribute("y");
    const XMLAttribute* wAttr = textElem->FindAttribute("w");
    const XMLAttribute* hAttr = textElem->FindAttribute("h");
    if (nameAttr && xAttr && wAttr && yAttr && hAttr)
    {
      std::string name = nameAttr->Value();
      int x = fromString<int>(xAttr->Value());
      int y = fromString<int>(yAttr->Value());
      int w = fromString<int>(wAttr->Value());
      int h = fromString<int>(hAttr->Value());

      monster.texture = name;
      monster.textureRect = sf::IntRect(x, y, w, h);
    }
    else if (nameAttr)
    {
      std::string name = nameAttr->Value();
      monster.texture = name;
      monster.textureRect = sf::IntRect(0, 0, 0, 0);
    }
  }

  const XMLElement* attrElem = monsterElement->FirstChildElement("attributes");
  if (attrElem)
  {
    for (const XMLElement* element = attrElem->FirstChildElement(); element; element = element->NextSiblingElement())
    {
      const XMLAttribute* nameAttr = element->FindAttribute("name");
      const XMLAttribute* valueAttr = element->FindAttribute("value");

      if (nameAttr && valueAttr)
      {
        std::string name = nameAttr->Value();
        int value = fromString<int>(valueAttr->Value());

        monster.attributeMap[name] = value;
      }
      else
      {
        TRACE("No nameAttr/valueAttr");
      }
    }
  }

  const XMLElement* actionElem = monsterElement->FirstChildElement("actions");
  if (actionElem)
  {
    for (const XMLElement* element = actionElem->FirstChildElement(); element; element = element->NextSiblingElement())
    {
      const XMLAttribute* nameAttr = element->FindAttribute("name");
      const XMLAttribute* chanceAttr = element->FindAttribute("chance");

      if (nameAttr && chanceAttr)
      {
        std::string name = nameAttr->Value();
        int chance = fromString<int>(chanceAttr->Value());
        std::string spell;

        if (name == "Spell" || name == "Summon")
        {
          spell = element->GetText();
        }

        MonsterActionEntry entry;
        entry.action = name;
        entry.objectName = spell;
        entry.weight = chance;

        monster.actions.push_back(entry);
      }
      else
      {
        TRACE("No nameAttr/chanceAttr");
      }
    }
  }

  const XMLElement* itemElem = monsterElement->FirstChildElement("items");
  if (itemElem)
  {
    for (const XMLElement* element = itemElem->FirstChildElement(); element; element = element->NextSiblingElement())
    {
      const XMLAttribute* nameAttr = element->FindAttribute("name");
      const XMLAttribute* chanceAttr = element->FindAttribute("chance");

      if (nameAttr && chanceAttr)
      {
        std::string name = nameAttr->Value();
        int chance = fromString<int>(chanceAttr->Value());

        MonsterDropItem drop;
        drop.itemName = name;
        drop.chance = chance;

        monster.itemDrop.push_back(drop);
      }
      else
      {
        TRACE("No nameAttr/chanceAttr");
      }
    }
  }

  const XMLElement* stealElem = monsterElement->FirstChildElement("steal");
  if (stealElem)
  {
    for (const XMLElement* element = stealElem->FirstChildElement(); element; element = element->NextSiblingElement())
    {
      std::string itemName = element->GetText();
      monster.stealItems.push_back(itemName);
    }
  }

  const XMLElement* resElem = monsterElement->FirstChildElement("resistance");
  if (resElem)
  {
    for (const XMLElement* element = resElem->FirstChildElement(); element; element = element->NextSiblingElement())
    {
      std::string name = element->FindAttribute("name")->Value();
      float resist = fromString<float>(element->FindAttribute("resist")->Value());

      monster.resistance[name] = resist;
    }
  }

  const XMLElement* imunElem = monsterElement->FirstChildElement("immunity");
  if (imunElem)
  {
    for (const XMLElement* element = imunElem->FirstChildElement(); element; element = element->NextSiblingElement())
    {
      std::string name = element->FindAttribute("name")->Value();
      monster.immunity.push_back(name);
    }
  }

  return monster;
}

void load_monsters()
{
  static const std::string database = config::res_path( "Monsters.xml");

  XMLDocument doc;
  if (doc.LoadFile(database.c_str()) != 0)
  {
    TRACE("Unable to open monster database %s (%s)!", database.c_str(), doc.GetErrorStr1());

    throw std::runtime_error("Unable to open monster database " + database);
  }

  const XMLElement* root = doc.FirstChildElement("monsters");

  for (const XMLElement* element = root->FirstChildElement(); element; element = element->NextSiblingElement())
  {
    MonsterDef monster = parse_monster_element(element);

    TRACE("Loaded monster %s", monster.name.c_str());

    monsters.push_back(monster);
  }
}

MonsterDef get_monster_definition(const std::string& name)
{
  for (auto it = monsters.begin(); it != monsters.end(); ++it)
  {
    if (it->name == name)
      return *it;
  }

  TRACE("No monster with name %s defined!", name.c_str());

  return monsters[0];
}

std::string get_monster_description(const std::string& name)
{
  return get_monster_definition(name).description;
}

std::vector<std::string> monster_drop_items(const MonsterDef& monster)
{
  std::vector<std::string> items;

  for (auto it = monster.itemDrop.begin(); it != monster.itemDrop.end(); ++it)
  {
    int rnd = random_range(0, 100);
    if (rnd <= it->chance)
    {
      items.push_back(it->itemName);
    }
  }

  return items;
}

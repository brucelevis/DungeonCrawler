#include <stdexcept>
#include <sstream>
#include <map>

#include "Utility.h"
#include "Config.h"
#include "logger.h"

#include "Game.h"
#include "Encounter.h"

#include "XMLHelpers.h"
using namespace tinyxml2;

static std::map<std::string, Encounter> _encounters;

Encounter::Encounter()
 : canEscape(true)
{

}

void Encounter::start() const
{
  Game::instance().startBattle(monsters, canEscape, music, script);
}

static std::vector<std::string> parse_group_element(const XMLElement* groupElement)
{
  std::vector<std::string> monsters;

  xml_for_each(groupElement,
    [&monsters](const XMLElement* element)
    {
      std::string elementName = element->Name();
      if (elementName == "monster")
      {
        std::string monsterName = xml_parse_attribute<std::string>::parse(element, "name");
        monsters.push_back(monsterName);
      }
    });

  return monsters;
}

static Encounter parse_encounter_element(const XMLElement* encounterElement)
{
  Encounter encounter;

  encounter.name = xml_parse_attribute<std::string>::parse(encounterElement, "name");
  encounter.music = xml_parse_attribute<std::string>::parse(encounterElement, "music");

  std::string canEscape = xml_parse_attribute<std::string>::parse(encounterElement, "canEscape");
  if (canEscape == "true" || canEscape.empty())
  {
    encounter.canEscape = true;
  }
  else if (canEscape == "false")
  {
    encounter.canEscape = false;
  }
  else
  {
    throw std::runtime_error("Parse error: canEscape should be true or false.");
  }

  for (const XMLElement* element = encounterElement->FirstChildElement(); element; element = element->NextSiblingElement())
  {
    std::string elementName = element->Name();

    if (elementName == "group")
    {
      encounter.monsters = parse_group_element(element);
    }
    else if (elementName == "script")
    {
      std::string script = element->GetText();
      std::istringstream ss{script};
      encounter.script = get_lines(ss);
    }
  }

  return encounter;
}

void load_encounters()
{
  static const std::string database = config::res_path( "Encounters.xml" );

  XMLDocument doc;
  if (doc.LoadFile(database.c_str()) != 0)
  {
    TRACE("Unable to open encounter database %s (%s)!", database.c_str(), doc.GetErrorStr1());

    throw std::runtime_error("Unable to open encounter database " + database);
  }

  const XMLElement* root = doc.FirstChildElement("encounters");
  for (const XMLElement* element = root->FirstChildElement(); element; element = element->NextSiblingElement())
  {
    std::string elementName = element->Name();

    if (elementName == "encounter")
    {
      Encounter encounter = parse_encounter_element(element);
      _encounters[encounter.name] = encounter;

      TRACE("Loaded encounter: %s", encounter.name.c_str());
    }
  }
}

const Encounter* get_encounter(const std::string& encounterName)
{
  auto it = _encounters.find(encounterName);

  if (it != _encounters.end())
  {
    return &it->second;
  }

  return 0;
}

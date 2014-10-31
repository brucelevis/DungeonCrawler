#include <algorithm>
#include <stdexcept>
#include <map>

#include "Config.h"
#include "logger.h"
#include "Skill.h"

#include "XMLHelpers.h"
using namespace tinyxml2;

static std::map<std::string, Skill> _skills;

const Skill& Skill::get(const std::string& name)
{
  auto it = _skills.find(name);

  if (it != _skills.end())
  {
    return it->second;
  }

  throw std::runtime_error("No skill " + name + " found!");
}

std::vector<std::string> Skill::getAllSkills()
{
  std::vector<std::string> skills;

  for (auto it = _skills.begin(); it != _skills.end(); ++it)
  {
    skills.push_back(it->first);
  }

  std::sort(skills.begin(), skills.end());

  return skills;
}

bool Skill::isSkill(const std::string& name)
{
  return _skills.count(name);
}

int Skill::getRanks(int percent) const
{
  return percent / ranks;
}

int Skill::getPercent(int rank) const
{
  return (100 / ranks) * rank;
}

void load_skills()
{
  static const std::string database = config::res_path( "Skills.xml" );

  XMLDocument doc;
  if (doc.LoadFile(database.c_str()) != 0)
  {
    TRACE("Unable to open skills database %s (%s)!", database.c_str(), doc.GetErrorStr1());

    throw std::runtime_error("Unable to open skills database " + database);
  }

  const XMLElement* root = doc.FirstChildElement("skills");

  xml_for_each(root, [](const XMLElement* element)
  {
    std::string elementName = element->Name();

    if (elementName == "skill")
    {
      std::string skillName = xml_parse_attribute<std::string>::parse(element, "name");
      int ranks = xml_parse_attribute<int>::parse(element, "ranks");
      int cost = xml_parse_attribute<int>::parse(element, "costOfRank");

      _skills[skillName].name = skillName;
      _skills[skillName].ranks = ranks;
      _skills[skillName].costOfRank = cost;

      TRACE("Loaded skill: %s", skillName.c_str());
    }
  });
}

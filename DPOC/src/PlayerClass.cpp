#include <stdexcept>

#include "logger.h"
#include "Utility.h"

#include "PlayerClass.h"

#include "../dep/tinyxml2.h"

using namespace tinyxml2;

static std::vector<PlayerClass> classes;

PlayerClass parse_class_element(const XMLElement* classElement)
{
  PlayerClass pc;
  pc.name = "ERROR";

  const XMLElement* nameElem = classElement->FirstChildElement("name");
  if (nameElem)
  {
    pc.name = nameElem->GetText();
  }

  const XMLElement* attributesElem = classElement->FirstChildElement("attributes");
  if (attributesElem)
  {
    for (const XMLElement* element = attributesElem->FirstChildElement(); element; element = element->NextSiblingElement())
    {
      const XMLAttribute* nameAttr = element->FindAttribute("name");
      const XMLAttribute* valueAttr = element->FindAttribute("value");

      if (nameAttr && valueAttr)
      {
        std::string name = nameAttr->Value();
        int value = fromString<int>(valueAttr->Value());

        pc.baseAttributes[name] = value;
      }
      else
      {
        TRACE("No nameAttr/valueAttr");
      }
    }
  }

  const XMLElement* spellsElem = classElement->FirstChildElement("spellsPerLevel");
  if (spellsElem)
  {
    for (const XMLElement* element = spellsElem->FirstChildElement(); element; element = element->NextSiblingElement())
    {
      int level = fromString<int>(element->FindAttribute("num")->Value());
      std::vector<std::string> spells;
      for (const XMLElement* spell = element->FirstChildElement(); spell; spell = spell->NextSiblingElement())
      {
        spells.push_back(spell->GetText());
      }
      pc.spells[level] = spells;
    }
  }

  const XMLElement* eqElem = classElement->FirstChildElement("equipment");
  if (eqElem)
  {
    for (const XMLElement* element = eqElem->FirstChildElement(); element; element = element->NextSiblingElement())
    {
      std::string item = element->GetText();
      pc.equipment.push_back(item);
    }
  }

  return pc;
}

void load_classes()
{
  static const std::string database = "Resources/Classes.xml";

  XMLDocument doc;
  if (doc.LoadFile(database.c_str()) != 0)
  {
    TRACE("Unable to open class database %s (%s)!", database.c_str(), doc.GetErrorStr1());

    throw std::runtime_error("Unable to open class database " + database);
  }

  const XMLElement* root = doc.FirstChildElement("classes");

  for (const XMLElement* element = root->FirstChildElement(); element; element = element->NextSiblingElement())
  {
    PlayerClass pclass = parse_class_element(element);

    TRACE("New class %s loaded.", pclass.name.c_str());

    classes.push_back(pclass);
  }
}

PlayerClass& player_class_ref(const std::string& className)
{
  for (auto it = classes.begin(); it != classes.end(); ++it)
  {
    if (it->name == className)
    {
      return *it;
    }
  }

  TRACE("No class %s defined", className.c_str());

  throw std::runtime_error("No class " + className + " defined");
}

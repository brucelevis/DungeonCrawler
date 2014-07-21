#include <stdexcept>

#include "Config.h"
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

  const XMLElement* descElem = classElement->FirstChildElement("description");
  if (descElem)
  {
    pc.description = descElem->GetText();
  }

  const XMLElement* effeElem = classElement->FirstChildElement("unarmedAttack");
  if (effeElem)
  {
    pc.unarmedAttackEffect = Effect::createFromXmlElement(effeElem);
  }

  const XMLElement* attributesElem = classElement->FirstChildElement("attributes");
  if (attributesElem)
  {
    for (const XMLElement* element = attributesElem->FirstChildElement(); element; element = element->NextSiblingElement())
    {
      const XMLAttribute* nameAttr = element->FindAttribute("name");
      const XMLAttribute* baseAttr = element->FindAttribute("base");
      const XMLAttribute* maxAttr = element->FindAttribute("max");

      if (nameAttr && baseAttr && maxAttr)
      {
        std::string name = nameAttr->Value();
        int base = fromString<int>(baseAttr->Value());
        int max = fromString<int>(maxAttr->Value());

        pc.baseAttributes[name].base = base;
        pc.baseAttributes[name].max  = max;
      }
      else
      {
        TRACE("No nameAttr/baseAttr/maxAttr");
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

  const XMLElement* startEqElem = classElement->FirstChildElement("startingEquipment");
  if (startEqElem)
  {
    for (const XMLElement* element = startEqElem->FirstChildElement(); element; element = element->NextSiblingElement())
    {
      std::string item = element->GetText();
      pc.startingEquipment.push_back(item);
    }
  }

  const XMLElement* textElem = classElement->FirstChildElement("texture");
  if (textElem)
  {
    std::string name = textElem->FindAttribute("name")->Value();
    int x = fromString<int>(textElem->FindAttribute("x")->Value());
    int y = fromString<int>(textElem->FindAttribute("y")->Value());
    int w = fromString<int>(textElem->FindAttribute("w")->Value());
    int h = fromString<int>(textElem->FindAttribute("h")->Value());
    pc.texture = name;
    pc.textureBlock.left = x;
    pc.textureBlock.top = y;
    pc.textureBlock.width = w;
    pc.textureBlock.height = h;
  }

  const XMLElement* faceElem = classElement->FirstChildElement("face");
  if (faceElem)
  {
    std::string name = faceElem->FindAttribute("name")->Value();
    int x = fromString<int>(faceElem->FindAttribute("x")->Value());
    int y = fromString<int>(faceElem->FindAttribute("y")->Value());
    int w = fromString<int>(faceElem->FindAttribute("w")->Value());
    int h = fromString<int>(faceElem->FindAttribute("h")->Value());
    pc.faceTexture = name;
    pc.textureRect = sf::IntRect(x, y, w, h);
  }

  const XMLElement* actionsElem = classElement->FirstChildElement("actions");
  if (actionsElem)
  {
    for (const XMLElement* element = actionsElem->FirstChildElement(); element; element = element->NextSiblingElement())
    {
      std::string actionName = element->GetText();
      pc.battleActions.push_back(actionName);
    }
  }
  else
  {
    // Default actions.
    pc.battleActions.push_back("Attack");
    pc.battleActions.push_back("Spell");
    pc.battleActions.push_back("Item");
    pc.battleActions.push_back("Guard");
    pc.battleActions.push_back("Run");
  }

  return pc;
}

void load_classes()
{
  static const std::string database = config::res_path("Classes.xml");

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

PlayerClass player_class_ref(const std::string& className)
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

std::vector<PlayerClass> get_all_classes()
{
  return classes;
}

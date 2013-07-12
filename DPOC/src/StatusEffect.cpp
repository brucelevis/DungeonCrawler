#include <vector>
#include <stdexcept>

#include "logger.h"
#include "Utility.h"
#include "StatusEffect.h"

#include "../dep/tinyxml2.h"

using namespace tinyxml2;

static std::vector<StatusEffect> statusEffects =
{
  {
    "Normal", "", "",
    sf::Color::White,
    false, 0, false,
    DAMAGE_NONE
  },

  {
    "Dead", "has fallen!", "comes back to life!",
    sf::Color::Red,
    false, 0, true,
    DAMAGE_NONE
  }
};

static DamageType damageTypeFromString(const std::string& type)
{
  if (type == "DAMAGE_NONE") return DAMAGE_NONE;
  if (type == "DAMAGE_FIXED") return DAMAGE_FIXED;
  if (type == "DAMAGE_PERCENT") return DAMAGE_PERCENT;

  TRACE("Unknown damage type %s", type.c_str());

  return DAMAGE_NONE;
}

static StatusEffect parse_status_effect_element(const XMLElement* statusElement)
{
  StatusEffect status;
  status.battleOnly = true;
  status.damagePerTurn = 0;
  status.damageType = DAMAGE_NONE;
  status.incapacitate = true;
  status.name = "ERROR";
  status.recoveryChance = 0;

  const XMLElement* nameElem = statusElement->FirstChildElement("name");
  const XMLElement* verbElem = statusElement->FirstChildElement("verb");
  const XMLElement* recovElem = statusElement->FirstChildElement("recoveryVerb");
  const XMLElement* colorElem = statusElement->FirstChildElement("color");
  const XMLElement* battElem = statusElement->FirstChildElement("battleOnly");
  const XMLElement* recovChanceElem = statusElement->FirstChildElement("recoveryChance");
  const XMLElement* incapElem = statusElement->FirstChildElement("incapacitate");
  const XMLElement* damageElem = statusElement->FirstChildElement("damage");
  const XMLElement* soundElem = statusElement->FirstChildElement("sound");

  if (nameElem)
    status.name = nameElem->GetText();
  if (verbElem)
    status.verb = verbElem->GetText();
  if (recovElem)
    status.recoverVerb = recovElem->GetText();
  if (colorElem)
  {
    int r = fromString<int>(colorElem->FindAttribute("r")->Value());
    int g = fromString<int>(colorElem->FindAttribute("g")->Value());
    int b = fromString<int>(colorElem->FindAttribute("b")->Value());
    status.color = sf::Color(r, g, b);
  }
  if (battElem)
    status.battleOnly = fromString<bool>(battElem->GetText());
  if (recovChanceElem)
    status.recoveryChance = fromString<int>(recovChanceElem->GetText());
  if (incapElem)
    status.incapacitate = fromString<bool>(incapElem->GetText());
  if (damageElem)
  {
    status.damageType = damageTypeFromString(damageElem->FindAttribute("type")->Value());
    status.damagePerTurn = fromString<int>(damageElem->FindAttribute("amount")->Value());
    status.damageStat = damageElem->FindAttribute("attr")->Value();
  }
  if (soundElem)
    status.sound = soundElem->GetText();

  return status;
}

void load_status_effects()
{
  static const std::string database = "Resources/StatusEffects.xml";

  XMLDocument doc;
  if (doc.LoadFile(database.c_str()) != 0)
  {
    TRACE("Unable to open status database %s (%s)!", database.c_str(), doc.GetErrorStr1());

    throw std::runtime_error("Unable to open status database " + database);
  }

  const XMLElement* root = doc.FirstChildElement("statusEffects");

  for (const XMLElement* element = root->FirstChildElement(); element; element = element->NextSiblingElement())
  {
    StatusEffect status = parse_status_effect_element(element);

    TRACE("New statusEffect %s loaded.", status.name.c_str());

    statusEffects.push_back(status);
  }
}

StatusEffect* get_status_effect(const std::string& status)
{
  for (auto it = statusEffects.begin(); it != statusEffects.end(); ++it)
  {
    if (to_lower(it->name) == to_lower(status))
    {
      return &(*it);
    }
  }

  TRACE("No status effect %s defined!", status.c_str());

  return 0;
}

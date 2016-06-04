#include <vector>
#include <stdexcept>

#include "Character.h"
#include "Config.h"
#include "logger.h"
#include "Utility.h"
#include "StatusEffect.h"

#include "XMLHelpers.h"
#include "../dep/tinyxml2.h"

using namespace tinyxml2;

static std::vector<StatusEffect> statusEffects =
{
  {
    "Normal", "", "",
    sf::Color::White,
    false, 0, false,
    DAMAGE_NONE,
    "", 0, 0, Effect { "", "" }
  },

  {
    "Dead", "has fallen!", "comes back to life!",
    sf::Color::Red,
    false, 0, true,
    DAMAGE_NONE,
    "", 0, 0, Effect { "", "" }
  }
};

int StatusEffect::applyDamage(Character* character) const
{
  int damage = 0;

  if (damageType == DAMAGE_FIXED)
  {
    damage = damagePerTurn;
  }
  else if (damageType == DAMAGE_PERCENT)
  {
    float percent = (float)damagePerTurn / 100.0f;
    damage = percent * (float)character->getAttribute(damageStat).max;

    if (damage == 0) damage = 1;
  }

  character->takeDamage(damageStat, damage);

  return damage;
}

static DamageType damageTypeFromString(const std::string& type)
{
  if (type == "DAMAGE_NONE") return DAMAGE_NONE;
  if (type == "DAMAGE_FIXED") return DAMAGE_FIXED;
  if (type == "DAMAGE_PERCENT") return DAMAGE_PERCENT;

  TRACE("Unknown damage type %s", type.c_str());

  return DAMAGE_NONE;
}

static StatusType statusTypeFromString(const std::string& type)
{
  if (type == "STATUS_NONE")    return STATUS_NONE;
  if (type == "STATUS_CONFUSE") return STATUS_CONFUSE;
  if (type == "STATUS_FUMBLE")  return STATUS_FUMBLE;
  if (type == "STATUS_BLIND")   return STATUS_BLIND;
  if (type == "STATUS_REFLECT") return STATUS_REFLECT;
  if (type == "STATUS_PROVOKE") return STATUS_PROVOKE;
  if (type == "STATUS_SILENCE") return STATUS_SILENCE;

  TRACE("Unknown status type=%s", type.c_str());

  return STATUS_NONE;
}

static StatusEffect parse_status_effect_element(const XMLElement* statusElement)
{
  StatusEffect status;
  status.battleOnly = true;
  status.damagePerTurn = 0;
  status.damageType = DAMAGE_NONE;
  status.incapacitate = false;
  status.name = "ERROR";
  status.recoveryChance = 0;
  status.statusType = STATUS_NONE;

  const XMLElement* nameElem = statusElement->FirstChildElement("name");
  const XMLElement* verbElem = statusElement->FirstChildElement("verb");
  const XMLElement* recovElem = statusElement->FirstChildElement("recoveryVerb");
  const XMLElement* colorElem = statusElement->FirstChildElement("color");
  const XMLElement* battElem = statusElement->FirstChildElement("battleOnly");
  const XMLElement* recovChanceElem = statusElement->FirstChildElement("recoveryChance");
  const XMLElement* incapElem = statusElement->FirstChildElement("incapacitate");
  const XMLElement* damageElem = statusElement->FirstChildElement("damage");
  const XMLElement* effeElem = statusElement->FirstChildElement("effect");

  if (valid_text_element(nameElem))
    status.name = nameElem->GetText();
  if (valid_text_element(verbElem))
    status.verb = verbElem->GetText();
  if (valid_text_element(recovElem))
    status.recoverVerb = recovElem->GetText();
  if (colorElem)
  {
    int r = fromString<int>(colorElem->FindAttribute("r")->Value());
    int g = fromString<int>(colorElem->FindAttribute("g")->Value());
    int b = fromString<int>(colorElem->FindAttribute("b")->Value());
    status.color = sf::Color(r, g, b);
  }
  if (valid_text_element(battElem))
    status.battleOnly = fromString<bool>(battElem->GetText());
  if (valid_text_element(recovChanceElem))
    status.recoveryChance = fromString<int>(recovChanceElem->GetText());
  if (valid_text_element(incapElem))
    status.incapacitate = fromString<bool>(incapElem->GetText());
  if (damageElem && damageElem->FindAttribute("type"))
  {
    status.damageType = damageTypeFromString(damageElem->FindAttribute("type")->Value());
    status.damagePerTurn = fromString<int>(damageElem->FindAttribute("amount")->Value());
    status.damageStat = damageElem->FindAttribute("attr")->Value();
  }
  if (effeElem)
    status.effect = Effect::createFromXmlElement(effeElem);

  const XMLElement* typesElem = statusElement->FirstChildElement("statusType");
  if (typesElem)
  {
    for (const XMLElement* element = typesElem->FirstChildElement(); element; element = typesElem->NextSiblingElement())
    {
      std::string type = element->GetText();

      status.statusType |= statusTypeFromString(type);
    }
  }

  return status;
}

void load_status_effects()
{
  static const std::string database = config::res_path("StatusEffects.xml");

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

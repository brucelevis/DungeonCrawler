#include <vector>
#include <stdexcept>

#include <BGL/Sound.h>
#include <BGL/Strings.h>
#include <BGL/logger.h>
#include <BGL/Random.h>

#include "Config.h"

#include "Attack.h"
#include "Message.h"
#include "Character.h"
#include "StatusEffect.h"
#include "Spell.h"

#include "../dep/tinyxml2.h"

using namespace tinyxml2;

static std::vector<Spell> spells;

static SpellType spellTypeFromString(const std::string& type)
{
  if (type == "SPELL_NONE") return SPELL_NONE;
  if (type == "SPELL_DAMAGE") return SPELL_DAMAGE;
  if (type == "SPELL_HEAL") return SPELL_HEAL;
  if (type == "SPELL_BUFF") return SPELL_BUFF;
  if (type == "SPELL_REMOVE_STATUS") return SPELL_REMOVE_STATUS;
  if (type == "SPELL_CAUSE_STATUS") return SPELL_CAUSE_STATUS;
  if (type == "SPELL_CUSTOM") return SPELL_CUSTOM;
  if (type == "SPELL_DRAIN") return SPELL_DRAIN;

  TRACE("Unknown spellType=%s", type.c_str());

  return SPELL_NONE;
}

static Spell parse_spell_element(const XMLElement* spellElement)
{
  Spell spell;
  spell.battleOnly = true;
  spell.mpCost = 0;
  spell.name = "ERROR";
  spell.description = "";
  spell.power = 0;
  spell.spellType = SPELL_NONE;
  spell.target = TARGET_NONE;
  spell.isPhysical = false;

  const XMLElement* nameElem = spellElement->FirstChildElement("name");
  const XMLElement* descElem = spellElement->FirstChildElement("description");
  const XMLElement* costElem = spellElement->FirstChildElement("cost");
  const XMLElement* targElem = spellElement->FirstChildElement("target");
  const XMLElement* battElem = spellElement->FirstChildElement("battleOnly");
  const XMLElement* powrElem = spellElement->FirstChildElement("power");
  const XMLElement* effeElem = spellElement->FirstChildElement("effect");
  const XMLElement* elemElem = spellElement->FirstChildElement("element");
  const XMLElement* verbElem = spellElement->FirstChildElement("verb");
  const XMLElement* physElem = spellElement->FirstChildElement("isPhysical");

  if (nameElem)
    spell.name = nameElem->GetText();
  if (descElem)
    spell.description = descElem->GetText();
  if (costElem)
    spell.mpCost = bgl::str::fromString<int>(costElem->GetText());
  if (targElem)
    spell.target = targetFromString(targElem->GetText());
  if (battElem)
    spell.battleOnly = bgl::str::fromString<bool>(battElem->GetText());
  if (powrElem)
    spell.power = bgl::str::fromString<int>(powrElem->GetText());
  if (effeElem)
    spell.effect = effeElem->GetText();
  if (elemElem)
    spell.element = elemElem->GetText();
  if (verbElem)
    spell.verb = verbElem->GetText();
  if (physElem)
    spell.isPhysical = bgl::str::fromString<bool>(physElem->GetText());

  const XMLElement* typeElem = spellElement->FirstChildElement("spellType");
  if (typeElem)
  {
    for (const XMLElement* element = typeElem->FirstChildElement(); element; element = element->NextSiblingElement())
    {
      SpellType type = spellTypeFromString(element->GetText());
      spell.spellType |= (int)type;
    }
  }

  const XMLElement* statusElem = spellElement->FirstChildElement("statusChange");
  if (statusElem)
  {
    for (const XMLElement* element = statusElem->FirstChildElement(); element; element = element->NextSiblingElement())
    {
      std::string name = element->FindAttribute("name")->Value();

      int chance = 100;
      const XMLAttribute* chanceAttr = element->FindAttribute("chance");
      if (chanceAttr)
      {
        chance = bgl::str::fromString<int>(chanceAttr->Value());
      }

      int duration = -1;
      const XMLAttribute* durAttr = element->FindAttribute("duration");
      if (durAttr)
      {
        duration = bgl::str::fromString<int>(durAttr->Value());
      }

      spell.causeStatus[name].chance = chance;
      spell.causeStatus[name].duration = duration;
    }
  }

  const XMLElement* buffElem = spellElement->FirstChildElement("buff");
  if (buffElem)
  {
    for (const XMLElement* element = buffElem->FirstChildElement(); element; element = element->NextSiblingElement())
    {
      std::string name = element->FindAttribute("name")->Value();
      spell.attributeBuffs.push_back(name);
    }
  }

  return spell;
}

void load_spells()
{
  static const std::string database = "Resources/Spells.xml";

  XMLDocument doc;
  if (doc.LoadFile(database.c_str()) != 0)
  {
    TRACE("Unable to open spell database %s (%s)!", database.c_str(), doc.GetErrorStr1());

    throw std::runtime_error("Unable to open spell database " + database);
  }

  const XMLElement* root = doc.FirstChildElement("spells");

  for (const XMLElement* element = root->FirstChildElement(); element; element = element->NextSiblingElement())
  {
    Spell spell = parse_spell_element(element);

    TRACE("New spell %s loaded.", spell.name.c_str());

    spells.push_back(spell);
  }
}

const Spell* get_spell(const std::string& spell)
{
  for (auto it = spells.begin(); it != spells.end(); ++it)
  {
    if (it->name == spell)
    {
      return &(*it);
    }
  }

  TRACE("ERROR: Trying to get nonexisting spell %s", spell.c_str());

  return 0;
}

int cast_spell(const Spell* spell, Character* caster, Character* target)
{
  int damage = 0;

  if ((spell->spellType & SPELL_DAMAGE) || (spell->spellType & SPELL_HEAL))
  {
    damage = calculate_magical_damage(caster, target, spell);
  }

  if ((spell->spellType & SPELL_DRAIN))
  {
    caster->getAttribute("hp").current += damage;
    clamp_attribute(caster->getAttribute("hp"));

    battle_message("%s drains life from %s!",
        caster->getName().c_str(), target->getName().c_str());
  }

  if (spell->spellType & SPELL_CAUSE_STATUS)
  {
    std::string soundToPlay;

    bool success = false;

    for (auto it = spell->causeStatus.begin(); it != spell->causeStatus.end(); ++it)
    {
      int range = bgl::rnd::random_range(0, 100);
      if (range < it->second.chance)
      {
        if (cause_status(target, it->first, false, it->second.duration))
        {
          success = true;
        }

        // Play the first applicable sound.
        if (soundToPlay.empty())
        {
          soundToPlay = get_status_effect(it->first)->sound;
        }
      }
    }

    if (!success && damage == 0)
    {
      battle_message("No effect...");
    }

    if (damage == 0 && !soundToPlay.empty())
    {
      bgl::play_sound("Resources/Audio/" + soundToPlay);
    }
  }

  if (spell->spellType & SPELL_REMOVE_STATUS)
  {
    std::vector<StatusEffect*> effects = target->getStatusEffects();

    for (auto it = spell->causeStatus.begin(); it != spell->causeStatus.end(); ++it)
    {
      cure_status(target, it->first);
    }

    // Play sound if the previous effects differ from current.
    if (damage == 0 && effects.size() != target->getStatusEffects().size())
    {
      bgl::play_sound(config::get("SOUND_RECOVERY"));
    }
  }

  if (spell->spellType & SPELL_BUFF)
  {
    for (auto it = spell->attributeBuffs.begin(); it != spell->attributeBuffs.end(); ++it)
    {
      buff(target, *it, spell->power);
    }

    if (damage == 0)
    {
      if (spell->power > 0)
      {
        bgl::play_sound(config::get("SOUND_BUFF"));
      }
      else if (spell->power < 0)
      {
        bgl::play_sound(config::get("SOUND_DEBUFF"));
      }
    }
  }

  target->takeDamage("hp", damage);

  return damage;
}

bool can_cast_spell(const Spell* spell, Character* caster)
{
  return spell->mpCost <= caster->getAttribute("mp").current;
}

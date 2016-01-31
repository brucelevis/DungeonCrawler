#include <vector>
#include <stdexcept>

#include "Config.h"
#include "Attack.h"
#include "logger.h"
#include "Character.h"
#include "Utility.h"
#include "Message.h"
#include "Vocabulary.h"
#include "Item.h"

#include "../dep/tinyxml2.h"

using namespace tinyxml2;

static std::vector<Item> itemDefinitions;

#define E_S(A, B) if (A == #B) return B

static ItemType itemTypeFromString(const std::string& type)
{
  E_S(type, ITEM_USE);
  E_S(type, ITEM_USE_MENU);
  E_S(type, ITEM_USE_BATTLE);
  E_S(type, ITEM_WEAPON);
  E_S(type, ITEM_SHIELD);
  E_S(type, ITEM_ARMOR);
  E_S(type, ITEM_HELMET);
  E_S(type, ITEM_MISC1);
  E_S(type, ITEM_MISC2);

  TRACE("Unknown item type %s", type.c_str());

  return ITEM_USE;
}

static ItemUseType itemUseTypeFromString(const std::string& type)
{
  E_S(type, ITEM_HEAL);
  E_S(type, ITEM_HEAL_FIXED);
  E_S(type, ITEM_RESTORE_MP);
  E_S(type, ITEM_RESTORE_MP_FIXED);
  E_S(type, ITEM_DAMAGE);
  E_S(type, ITEM_BUFF);
  E_S(type, ITEM_REMOVE_STATUS);
  E_S(type, ITEM_CAUSE_STATUS);
  E_S(type, ITEM_CUSTOM);

  TRACE("Unknown item use type %s", type.c_str());

  return ITEM_HEAL;
}

static Item parse_item_element(const XMLElement* itemElement)
{
  Item item;
  item.cost = 0;
  item.name = "ERROR";
  item.itemUseType = ITEM_HEAL;
  item.stackSize = 0;
  item.target = TARGET_NONE;
  item.type = ITEM_USE;

  const XMLElement* nameElem = itemElement->FirstChildElement("name");
  const XMLElement* descElem = itemElement->FirstChildElement("description");
  const XMLElement* costElem = itemElement->FirstChildElement("cost");
  const XMLElement* typeElem = itemElement->FirstChildElement("type");
  const XMLElement* targElem = itemElement->FirstChildElement("target");
  const XMLElement* useElem  = itemElement->FirstChildElement("onUse");
  const XMLElement* statElem = itemElement->FirstChildElement("status");
  const XMLElement* effeElem = itemElement->FirstChildElement("effect");
  const XMLElement* verbElem = itemElement->FirstChildElement("verb");
  const XMLElement* formElem = itemElement->FirstChildElement("formula");
  const XMLElement* singleElem = itemElement->FirstChildElement("element");

  if (nameElem)
    item.name = nameElem->GetText();
  if (descElem)
    item.description = descElem->GetText();
  if (costElem)
    item.cost = fromString<int>(costElem->GetText());
  if (typeElem)
    item.type = itemTypeFromString(typeElem->GetText());
  if (targElem)
    item.target = targetFromString(targElem->GetText());
  if (useElem)
    item.itemUseType = itemUseTypeFromString(useElem->GetText());
  if (statElem)
    item.status.push_back(statElem->GetText());
  if (formElem)
    item.formula = formElem->GetText();
  if (effeElem)
  {
    item.effect = Effect::createFromXmlElement(effeElem);
  }
  if (verbElem)
    item.useVerb = verbElem->GetText();
  if (singleElem)
  {
    // <element>...</element>
    // Typically used for weapons that only has an element type.
    std::string elementName = singleElem->GetText();
    item.elements[elementName] = 1;
  }

  const XMLElement* attrElem = itemElement->FirstChildElement("attributes");
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

        item.attributeGain[name] = value;
      }
      else
      {
        TRACE("Attribute name or value not set for item %s!", item.name.c_str());

        throw std::runtime_error("Attribute name or value not set for item " + item.name);
      }
    }
  }

  const XMLElement* elemElement = itemElement->FirstChildElement("elements");
  if (elemElement)
  {
    for (const XMLElement* element = elemElement->FirstChildElement(); element; element = element->NextSiblingElement())
    {
      std::string name = element->FindAttribute("name")->Value();
      float value = fromString<float>(element->FindAttribute("value")->Value());
      item.elements[name] = value;
    }
  }

  // List of status effects: <statusEffects> <status>Dead</status> ... </statusEffects>
  const XMLElement* statusEffectsElement = itemElement->FirstChildElement("statusEffects");
  if (statusEffectsElement)
  {
    for (const XMLElement* element = statusEffectsElement->FirstChildElement(); element; element = element->NextSiblingElement())
    {
      std::string statusName = element->GetText();

      item.status.push_back(statusName);
    }
  }

  return item;
}

void load_items()
{
  static const std::string itemDatabase = config::res_path("Items.xml");

  XMLDocument doc;

  if (doc.LoadFile(itemDatabase.c_str()) != 0)
  {
    TRACE("Unable to open item database %s (%s)!", itemDatabase.c_str(), doc.GetErrorStr1());

    throw std::runtime_error("Unable to open item database " + itemDatabase);
  }

  const XMLElement* root = doc.FirstChildElement("items");
  for (const XMLElement* element = root->FirstChildElement(); element; element = element->NextSiblingElement())
  {
    Item item = parse_item_element(element);

    TRACE("Loaded new item %s", item.name.c_str());

    itemDefinitions.push_back(item);
  }
}

Item create_item(const std::string& name, int stackSize)
{
  for (auto it = itemDefinitions.begin(); it != itemDefinitions.end(); ++it)
  {
    if (to_lower(it->name) == to_lower(name))
    {
      Item itemCopy = *it;
      itemCopy.stackSize = stackSize;
      return itemCopy;
    }
  }

  return Item();
}

Item& item_ref(const std::string& name)
{
  for (auto it = itemDefinitions.begin(); it != itemDefinitions.end(); ++it)
  {
    if (to_lower(it->name) == to_lower(name))
      return *it;
  }

  TRACE("No item %s defined!", name.c_str());

  throw std::runtime_error("No item " + name + " defined!");
}

int use_item(Item* item, Character* user, Character* target)
{
  int damage = calculate_physical_damage_item(user, target, item);

  if (item->itemUseType == ITEM_HEAL || item->itemUseType == ITEM_HEAL_FIXED ||
          item->itemUseType == ITEM_DAMAGE)
  {
    target->takeDamage(terms::hp, damage);
  }
  else if (item->itemUseType == ITEM_RESTORE_MP || item->itemUseType == ITEM_RESTORE_MP_FIXED)
  {
    target->takeDamage(terms::mp, damage);

    battle_message("%s's %s restored by %d!",
        target->getName().c_str(), vocab_mid(terms::mp).c_str(), damage);

    damage = 0;
  }

  for (auto it = item->attributeGain.begin(); it != item->attributeGain.end(); ++it)
  {
    if (item->itemUseType == ITEM_BUFF)
    {
      target->getAttribute(it->first).max += it->second;
      reset_attribute(target->getAttribute(it->first));

      show_message("%s's %s increased by %d!",
          target->getName().c_str(), vocab(it->first.c_str()).c_str(), it->second);
    }
  }

  if (item->itemUseType == ITEM_REMOVE_STATUS)
  {
    for (const auto& statusName : item->status)
    {
      cure_status(target, statusName);

      // If dead they should be restored with HP.
      if (statusName == "Dead")
      {
        target->getAttribute(terms::hp).current = 1;
      }
    }
  }
  else if (item->itemUseType == ITEM_CAUSE_STATUS)
  {
    bool nothingHappened = true;

    for (const auto& statusName : item->status)
    {
      if (cause_status(target, statusName, false))
      {
        nothingHappened = false;
      }
    }

    if (nothingHappened)
    {
      battle_message("No effect...");
    }
  }

  return damage;
}

std::string equip_type_string(ItemType itemType)
{
  if (itemType == ITEM_WEAPON) return vocab(terms::weapon);
  else if (itemType == ITEM_SHIELD) return vocab(terms::shield);
  else if (itemType == ITEM_ARMOR) return vocab(terms::armour);
  else if (itemType == ITEM_HELMET) return vocab(terms::helmet);
  else if (itemType == ITEM_MISC1) return vocab(terms::misc1);
  else if (itemType == ITEM_MISC2) return vocab(terms::misc2);
  return "";
}

std::vector<std::string> get_equip_names()
{
  return std::vector<std::string>
  {
    terms::weapon,
    terms::shield,
    terms::armour,
    terms::helmet,
    terms::misc1,
    terms::misc2
  };
}


#include <fstream>
#include <sstream>
#include <vector>
#include <map>
#include <stdexcept>

#include <BGL/logger.h>
#include <BGL/Strings.h>

#include "Direction.h"

#include "Game.h"
#include "Entity.h"
#include "Map.h"
#include "Player.h"
#include "PlayerCharacter.h"
#include "Character.h"
#include "Persistent.h"

#include "SaveLoad.h"

#include "../dep/tinyxml2.h"

using namespace tinyxml2;

static void parseMapElement(const XMLElement* mapElement);
static EntityData parseEntityElement(const XMLElement* entityElement);
static void parsePlayerElement(const XMLElement* playerElement);
static void parsePartyElement(const XMLElement* partyElement);
static CharacterData parseCharacterElement(const XMLElement* characterElement);
static void parseInventoryElement(const XMLElement* inventoryElement);
static void parsePersistents(const XMLElement* persElement);

void save_game(const std::string& saveFile)
{
  TRACE("Saving game to: %s", saveFile.c_str());

  std::ostringstream xml;
  xml << "<save>\n";

  xml << Game::instance().getCurrentMap()->xmlDump();
  xml << get_player()->xmlDump();
  xml << Persistent<int>::instance().xmlDump();

  xml << "</save>\n";

  TRACE("XmlSave:\n%s", xml.str().c_str());

  std::ofstream ofile("Resources/Saves/" + saveFile);
  ofile << xml.str();
  ofile.close();
}

void load_game(const std::string& saveFile)
{
  std::string fullPath = "Resources/Saves/" + saveFile;

  TRACE("Loading game: %s", fullPath.c_str());

  XMLDocument doc;
  if (doc.LoadFile(fullPath.c_str()) != 0)
  {
    TRACE("Unable to load save: %s (%s)", fullPath.c_str(), doc.GetErrorStr1());

    throw std::runtime_error("Unable to load save: " +
        fullPath + "(" + std::string(doc.GetErrorStr1()) + ")");
  }

  const XMLElement* root = doc.FirstChildElement("save");

  for (const XMLElement* element = root->FirstChildElement(); element; element = element->NextSiblingElement())
  {
    std::string elementName = element->Name();

    if (elementName == "map")
    {
      parseMapElement(element);
    }
    else if (elementName == "player")
    {
      parsePlayerElement(element);
    }
    else if (elementName == "persistents")
    {
      parsePersistents(element);
    }
  }

  TRACE("Loading completed.");
}

CharacterData get_party_leader_from_save(const std::string& saveFile)
{
  std::string fullPath = "Resources/Saves/" + saveFile;

  XMLDocument doc;
  if (doc.LoadFile(fullPath.c_str()) != 0)
  {
    TRACE("Unable to load save: %s (%s)", fullPath.c_str(), doc.GetErrorStr1());

    throw std::runtime_error("Unable to load save: " +
        fullPath + "(" + std::string(doc.GetErrorStr1()) + ")");
  }

  const XMLElement* root = doc.FirstChildElement("save");

  for (const XMLElement* element = root->FirstChildElement(); element; element = element->NextSiblingElement())
  {
    std::string elementName = element->Name();

    if (elementName == "player")
    {
      for (const XMLElement* pElem = element->FirstChildElement(); pElem; pElem = pElem->NextSiblingElement())
      {
        elementName = pElem->Name();

        if (elementName == "party")
        {
          std::vector<EntityData> entityData;
          std::vector<CharacterData> characterData;

          for (const XMLElement* cElem = pElem->FirstChildElement(); cElem; cElem = cElem->NextSiblingElement())
          {
            CharacterData charData = parseCharacterElement(cElem->FirstChildElement("playerCharacter"));

            return charData;
          }

        }
      }
    }
  }

  return CharacterData();
}

void parseMapElement(const XMLElement* mapElement)
{
  TRACE("Parse Map Element");

  const XMLAttribute* nameAttrib  = mapElement->FindAttribute("name");
  if (nameAttrib)
  {
    TRACE(" - name=%s", nameAttrib->Value());

    Game::instance().loadNewMap(nameAttrib->Value());
  }

  const XMLElement* entElem = mapElement->FirstChildElement("entities");
  if (entElem)
  {
    for (entElem = entElem->FirstChildElement(); entElem; entElem = entElem->NextSiblingElement())
    {
      if (std::string(entElem->Name()) != "entity")
        continue;

      EntityData data = parseEntityElement(entElem);

      std::vector<Entity*> entities = Game::instance().getCurrentMap()->getEntities();

      for (auto it = entities.begin(); it != entities.end(); ++it)
      {
        if ((*it)->getTag() == data.tag)
        {
          (*it)->setPosition(data.x, data.y);
          (*it)->setDirection(data.dir);
          (*it)->setWalkSpeed(data.speed);
          (*it)->setWalkThrough(data.walkThrough);
        }
      }
    }
  }
}

EntityData parseEntityElement(const XMLElement* entityElement)
{
  TRACE("Parse Entity Element");

  EntityData data;

  const XMLAttribute* nameAttrib = entityElement->FindAttribute("name");
  const XMLAttribute* tagAttrib = entityElement->FindAttribute("tag");
  const XMLElement* xElem = entityElement->FirstChildElement("x");
  const XMLElement* yElem = entityElement->FirstChildElement("y");
  const XMLElement* dirElem = entityElement->FirstChildElement("direction");
  const XMLElement* walkThroughElemt = entityElement->FirstChildElement("walkThrough");
  const XMLElement* speedElem = entityElement->FirstChildElement("speed");
  const XMLElement* spriteElem = entityElement->FirstChildElement("sprite");

  data.name = nameAttrib->Value();
  data.tag = tagAttrib->Value();
  data.x = bgl::str::fromString<int>(xElem->GetText());
  data.y = bgl::str::fromString<int>(yElem->GetText());
  data.dir = directionFromString(dirElem->GetText());
  data.walkThrough = bgl::str::fromString<bool>(walkThroughElemt->GetText());
  data.speed = bgl::str::fromString<float>(speedElem->GetText());
  data.spriteName = spriteElem->FindAttribute("name")->Value();

  TRACE(" - name=%s", data.name.c_str());
  TRACE(" - tag=%s", data.tag.c_str());
  TRACE(" - x=%d", data.x);
  TRACE(" - y=%d", data.y);
  TRACE(" - dir=%s", directionToString(data.dir).c_str());
  TRACE(" - walkThrough=%s", data.walkThrough ? "true" : "false");
  TRACE(" - speed=%f", data.speed);
  TRACE(" - sprite=%s", data.spriteName.c_str());

  return data;
}

void parsePlayerElement(const XMLElement* playerElement)
{
  TRACE("Parse Player Element");

  for (const XMLElement* element = playerElement->FirstChildElement(); element; element = element->NextSiblingElement())
  {
    std::string elemName = element->Name();

    if (elemName == "party")
    {
      parsePartyElement(element);
    }
    else if (elemName == "inventory")
    {
      parseInventoryElement(element);
    }
    else if (elemName == "gold")
    {
      int gold = bgl::str::fromString<int>(element->GetText());

      TRACE(" - gold=%d", gold);

      get_player()->gainGold(gold);
    }
  }
}

void parsePartyElement(const XMLElement* partyElement)
{
  TRACE("Parse Party Element");

  std::vector<EntityData> entityData;
  std::vector<CharacterData> characterData;

  for (const XMLElement* element = partyElement->FirstChildElement(); element; element = element->NextSiblingElement())
  {
    const XMLAttribute* idAttrib = element->FindAttribute("id");

    EntityData entData = parseEntityElement(element->FirstChildElement("entity"));
    CharacterData charData = parseCharacterElement(element->FirstChildElement("playerCharacter"));

    entityData.push_back(entData);
    characterData.push_back(charData);
  }

  std::vector<EntityData*> ePtr;
  std::vector<CharacterData*> cPtr;

  for (size_t i = 0; i < entityData.size(); i++)
  {
    ePtr.push_back(&entityData[i]);
    cPtr.push_back(&characterData[i]);
  }

  Game::instance().setPlayer(Player::createFromSaveData(cPtr, ePtr));
}

CharacterData parseCharacterElement(const XMLElement* characterElement)
{
  TRACE("Parse Character Element");

  CharacterData data;

  std::string name = characterElement->FindAttribute("name")->Value();
  data.name = name;

  TRACE(" - name=%s", name.c_str());

  const XMLElement* equipElement = characterElement->FirstChildElement("equipment");
  for (const XMLElement* element = equipElement->FirstChildElement(); element; element = element->NextSiblingElement())
  {
    std::string eqName = element->Name();
    std::string itemName = element->GetText();

    TRACE(" - equipment[%s]=%s", eqName.c_str(), itemName.c_str());

    data.equipment[eqName] = itemName;
  }

  const XMLElement* spellsElement = characterElement->FirstChildElement("spells");
  for (const XMLElement* element = spellsElement->FirstChildElement(); element; element = element->NextSiblingElement())
  {
    std::string spellName = element->GetText();

    TRACE(" - spell=%s", spellName.c_str());

    data.spells.push_back(spellName);
  }

  const XMLElement* classElement = characterElement->FirstChildElement("class");
  std::string className = classElement->GetText();
  data.className = className;

  TRACE(" - className=%s", className.c_str());

  const XMLElement* textureElement = characterElement->FirstChildElement("texture");
  std::string textureName = textureElement->FindAttribute("name")->Value();
  int tx = bgl::str::fromString<int>(textureElement->FindAttribute("x")->Value());
  int ty = bgl::str::fromString<int>(textureElement->FindAttribute("y")->Value());
  int tw = bgl::str::fromString<int>(textureElement->FindAttribute("w")->Value());
  int th = bgl::str::fromString<int>(textureElement->FindAttribute("h")->Value());
  data.textureName = textureName;
  data.textureX = tx;
  data.textureY = ty;
  data.textureW = tw;
  data.textureH = th;

  TRACE(" - texture=%s, [%d, %d, %d, %d", textureName.c_str(), tx, ty, tw, th);

  const XMLElement* attribElement = characterElement->FirstChildElement("attributes");
  for (const XMLElement* element = attribElement->FirstChildElement(); element; element = element->NextSiblingElement())
  {
    std::string attribName = element->FindAttribute("name")->Value();
    int current = bgl::str::fromString<int>(element->FindAttribute("current")->Value());
    int max = bgl::str::fromString<int>(element->FindAttribute("max")->Value());

    TRACE(" - attribute[%s]=[%d/%d]", attribName.c_str(), current, max);

    data.attributes[attribName].current = current;
    data.attributes[attribName].max = max;
  }

  const XMLElement* statusElement = characterElement->FirstChildElement("statusEffects");
  for (const XMLElement* element = statusElement->FirstChildElement(); element; element = element->NextSiblingElement())
  {
    std::string statusName = element->GetText();

    TRACE(" - statusEffect=%s", statusName.c_str());

    data.statusEffects.push_back(statusName);
  }

  return data;
}

void parseInventoryElement(const XMLElement* inventoryElement)
{
  TRACE("Parse Inventory Element");

  for (const XMLElement* element = inventoryElement->FirstChildElement(); element; element = element->NextSiblingElement())
  {
    const XMLAttribute* nameAttrib = element->FindAttribute("name");
    const XMLAttribute* stackAttrib = element->FindAttribute("stackSize");

    std::string name = nameAttrib->Value();
    int stackSize = bgl::str::fromString<int>(stackAttrib->Value());

    TRACE(" - ItemName=%s, stackSize=%d", name.c_str(), stackSize);

    get_player()->addItemToInventory(name, stackSize);
  }
}

void parsePersistents(const XMLElement* persElement)
{
  TRACE("Parse Persistents");

  for (const XMLElement* element = persElement->FirstChildElement(); element; element = element->NextSiblingElement())
  {
    std::string key = element->FindAttribute("key")->Value();
    int value = bgl::str::fromString<int>(element->FindAttribute("value")->Value());

    TRACE(" - key=%s, value=%d", key.c_str(), value);

    Persistent<int>::instance().set(key, value);
  }
}

#include <string>
#include <vector>
#include <map>

#include "Utility.h"
#include "logger.h"
#include "Config.h"

#include "Game.h"
#include "Player.h"
#include "PlayerCharacter.h"
#include "PlayerClass.h"
#include "SceneManager.h"

#include "BattleTest.h"

#include "../dep/tinyxml2.h"

using namespace tinyxml2;

void start_test_battle()
{
  XMLDocument doc;
  doc.LoadFile(config::res_path("BattleTest.xml").c_str());

  TRACE("*** STARTING TEST BATTLE *** ");

  const XMLElement* root = doc.FirstChildElement();

  std::vector<std::string> monsters;
  Player* player = Player::createBlank();

  for (const XMLElement* elem = root->FirstChildElement(); elem; elem = elem->NextSiblingElement())
  {
    std::string name = elem->Name();

    if (name == "character")
    {
      std::string charName = elem->FindAttribute("name")->Value();
      std::string charClass = elem->FindAttribute("class")->Value();
      int level = fromString<int>(elem->FindAttribute("level")->Value());

      std::map<std::string, std::string> eqMap;

      for (const XMLElement* eq = elem->FirstChildElement(); eq; eq = eq->NextSiblingElement())
      {
        eqMap[eq->Name()] = eq->GetText();
      }

      TRACE(" NEW CHARACTER [%s, %s]", charName.c_str(), charClass.c_str());

      player->addNewCharacter(charName, charClass, 0, 0, level);
      for (auto it = eqMap.begin(); it != eqMap.end(); ++it)
      {
        TRACE("  EQUIP %s with [%s : %s]", charName.c_str(), it->first.c_str(), it->second.c_str());
        player->getParty().back()->equip(it->first, it->second);
      }
    }
    else if (name == "monsters")
    {
      monsters = split_string(elem->GetText(), ',');

      TRACE(" MONSTERS: %s", elem->GetText());
    }
    else if (name == "items")
    {
      for (const XMLElement* eq = elem->FirstChildElement(); eq; eq = eq->NextSiblingElement())
      {
        std::string item = eq->FindAttribute("name")->Value();
        int quant = fromString<int>(eq->FindAttribute("amount")->Value());
        player->addItemToInventory(item, quant);

        TRACE(" Adding %d %s to player inventory.", quant, item.c_str());
      }
    }
  }

  Game::instance().setPlayer(player);
  Game::instance().startBattle(monsters, false);
  SceneManager::instance().run();
}

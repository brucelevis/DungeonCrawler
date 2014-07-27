#include <cstdlib>
#include <string>

#include "SceneManager.h"

#include "logger.h"
#include "SaveLoad.h"
#include "BattleTest.h"

#include "Config.h"
#include "draw_text.h"
#include "Spell.h"
#include "Item.h"
#include "Monster.h"
#include "PlayerClass.h"
#include "StatusEffect.h"
#include "Encounter.h"

#include "TitleScreen.h"

#include "Player.h"
#include "Game.h"

int main(int argc, char* argv[])
{
  START_LOG;

  init_text_drawing();

  config::load_config();

  // Load databases.
  load_spells();
  load_items();
  load_monsters();
  load_classes();
  load_status_effects();
  load_encounters();

  srand(time(0));

  SceneManager::instance().create();

  if (argc > 1)
  {
    std::string arg = argv[1];
    if (arg == "test")
    {
      start_test_battle();

      return 0;
    }
  }

//  SceneManager::instance().addScene(&Game::instance());

  SceneManager::instance().addScene(new TitleScreen);
  SceneManager::instance().run();

  return 0;
}

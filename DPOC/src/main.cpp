#include <cstdlib>
#include <string>

#include "SaveLoad.h"
#include "BattleTest.h"

#include "Config.h"
#include "Spell.h"
#include "Item.h"
#include "Monster.h"
#include "PlayerClass.h"
#include "StatusEffect.h"

#include "TitleScreen.h"

#include "Player.h"
#include "Editor.h"
#include "Game.h"

#include <BGL/Text.h>
#include <BGL/logger.h>
#include <BGL/SceneManager.h>

int main(int argc, char* argv[])
{
  START_LOG;

  bgl::init_text_drawing(new bgl::Font("Resources/font_8x8.png", 8, 8, 0));

  config::load_config();

  // Load databases.
  load_spells();
  load_items();
  load_monsters();
  load_classes();
  load_status_effects();

  srand(time(0));

  bgl::SceneManager::instance().create(config::GAME_RES_X, config::GAME_RES_Y, 2, config::FPS);

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

  bgl::SceneManager::instance().addScene(new TitleScreen);
  bgl::SceneManager::instance().run();

  return 0;
}

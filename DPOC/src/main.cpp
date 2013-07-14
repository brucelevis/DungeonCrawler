#include <cstdlib>
#include <string>

#include "logger.h"
#include "SaveLoad.h"

#include "Config.h"
#include "draw_text.h"
#include "Spell.h"
#include "Item.h"
#include "Monster.h"
#include "PlayerClass.h"
#include "StatusEffect.h"

#include "Player.h"
#include "Editor.h"
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

  srand(time(0));

  if (argc > 1)
  {
    std::string arg = argv[1];
    if (arg == "e")
    {
      Editor editor;
      editor.run();
    }
    else if (arg == "l")
    {
      std::string saveFile = argv[2];

      load_game(saveFile);

      Game::instance().run();
    }
  }
  else
  {
    Game::instance().setPlayer(Player::create());
    Game::instance().run();
  }

  return 0;
}

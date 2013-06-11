#include <cstdlib>
#include <string>

#include "logger.h"

#include "draw_text.h"
#include "Editor.h"
#include "Game.h"

int main(int argc, char* argv[])
{
  START_LOG;

  init_text_drawing();

  srand(time(0));

  if (argc > 1)
  {
    std::string arg = argv[1];
    if (arg == "e")
    {
      Editor editor;
      editor.run();
    }
    else
    {
      Game::instance().run();
    }
  }
  else
  {
    Game::instance().run();
  }

  return 0;
}

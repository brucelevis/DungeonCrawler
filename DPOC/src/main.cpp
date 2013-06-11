#include <cstdlib>

#include "logger.h"

#include "draw_text.h"
#include "Editor.h"
#include "Game.h"

int main()
{
  START_LOG;

  init_text_drawing();

  srand(time(0));

//  Editor editor;
//  editor.run();

  Game::instance().run();

  return 0;
}

#include <cstdarg>

#include "logger.h"
#include "draw_text.h"

static sf::Font font;
static sf::Text text;

void init_text_drawing()
{
  TRACE("Initialize text drawing.");

  if (!font.loadFromFile("Resources/arial.ttf"))
  {
    TRACE("Unable to load font Resources/arial.ttf");
  }

  text.setFont(font);
  text.setColor(sf::Color::White);
  text.setCharacterSize(14);
}

void draw_text(sf::RenderTarget& target, int x, int y, const char* fmt, ...)
{
  char buffer[512];

  va_list args;
  va_start(args, fmt);

  vsprintf(buffer, fmt, args);

  va_end(args);

  text.setString(buffer);
  text.setPosition(x, y);

  target.draw(text);
}

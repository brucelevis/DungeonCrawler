#include <cstdarg>

#include "logger.h"
#include "draw_text.h"
#include "Cache.h"

static sf::Font font;
static sf::Text text;
static sf::Texture* bmpFont;

static void draw_bmp_text(sf::RenderTarget& target, int x, int y, const sf::Color& color, const std::string& str)
{
  sf::Sprite sprite;
  sprite.setTexture(*bmpFont);

  int px = x;
  int py = y;
  for (size_t i = 0; i < str.size(); i++)
  {
    char c = str[i];

    if (c != '\n')
    {
      int charX = 8 * (c % (bmpFont->getSize().x / 8));
      int charY = 8 * (c / (bmpFont->getSize().x / 8));

      sprite.setTextureRect(sf::IntRect(charX, charY, 8, 8));
      sprite.setPosition(px, py);
      sprite.setColor(color);
      target.draw(sprite);

      px += 8;
    }
    else
    {
      px = x;
      py += 8;
    }
  }
}

void init_text_drawing()
{
  TRACE("Initialize text drawing.");

  if (!font.loadFromFile("Resources/UI/arial.ttf"))
  {
    TRACE("Unable to load font Resources/UI/arial.ttf");
  }

  text.setFont(font);
  text.setColor(sf::Color::White);
  text.setCharacterSize(14);

  bmpFont = cache::loadTexture("Resources/UI/font_8x8.png");
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

void draw_text_bmp(sf::RenderTarget& target, int x, int y, const char* fmt, ...)
{
  char buffer[512];

  va_list args;
  va_start(args, fmt);

  vsprintf(buffer, fmt, args);

  va_end(args);

  draw_bmp_text(target, x, y, sf::Color::White, buffer);
}

void draw_text_bmp_ex(sf::RenderTarget& target, int x, int y, const sf::Color& color, const char* fmt, ...)
{
  char buffer[512];

  va_list args;
  va_start(args, fmt);

  vsprintf(buffer, fmt, args);

  va_end(args);

  draw_bmp_text(target, x, y, color, buffer);
}

#include <cstdarg>

#include "Config.h"
#include "logger.h"
#include "draw_text.h"
#include "Cache.h"

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
      py += 9;
    }
  }
}

void init_text_drawing()
{
  TRACE("Initialize text drawing.");

  bmpFont = cache::loadTexture("UI/font_8x8.png");
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

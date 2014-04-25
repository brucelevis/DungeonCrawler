#ifndef DRAW_TEXT_H
#define DRAW_TEXT_H

#include <SFML/Graphics.hpp>

void init_text_drawing();

void draw_text(sf::RenderTarget& target, int x, int y, const char* fmt, ...);
void draw_text_bmp(sf::RenderTarget& target, int x, int y, const char* fmt, ...);
void draw_text_bmp_ex(sf::RenderTarget& target, int x, int y, const sf::Color& color, const char* fmt, ...);

#endif

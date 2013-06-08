#ifndef DRAW_TEXT_H
#define DRAW_TEXT_H

#include <SFML/Graphics.hpp>

void init_text_drawing();

void draw_text(sf::RenderTarget& target, int x, int y, const char* fmt, ...);

#endif

#ifndef FRAME_H
#define FRAME_H

#include <SFML/Graphics.hpp>

void draw_frame(sf::RenderTarget& target, int x, int y, int w, int h);
void draw_frame(sf::RenderTarget& target, int x, int y, int w, int h, int thickness);

void draw_gui_frame(sf::RenderTarget& target, int x, int y, int w, int h);

sf::RectangleShape make_select_rect(int x, int y, int w, int h, sf::Color color = sf::Color::Red);

#endif

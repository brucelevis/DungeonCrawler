#ifndef DRAWING_H_
#define DRAWING_H_

#include <SFML/Graphics.hpp>

void draw_texture(sf::RenderTarget& target, const sf::Texture* texture, int x, int y);
void draw_texture(sf::RenderTarget& target, const sf::Texture* texture, int src_x, int src_y, int src_w, int src_h, int x, int y, const sf::Color& color = sf::Color::White);

void draw_rectangle(sf::RenderTarget& target, int x, int y, int w, int h, const sf::Color& color);

#endif /* DRAWING_H_ */

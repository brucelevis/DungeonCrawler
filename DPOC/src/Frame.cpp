#include "Frame.h"

void draw_frame(sf::RenderTarget& target, int x, int y, int w, int h)
{
  sf::RectangleShape rect;
  rect.setSize(sf::Vector2f(w - 4, h - 4));
  rect.setPosition(2 + x, 2 + y);
  rect.setFillColor(sf::Color::Black);
  rect.setOutlineColor(sf::Color::White);
  rect.setOutlineThickness(2.0f);
  target.draw(rect);
}

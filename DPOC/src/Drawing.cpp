#include "Drawing.h"

void draw_texture(sf::RenderTarget& target, const sf::Texture* texture, int x, int y)
{
  sf::Sprite sprite;
  sprite.setTexture(*texture);
  sprite.setPosition(x, y);
  target.draw(sprite);
}

void draw_texture(sf::RenderTarget& target, const sf::Texture* texture, int src_x, int src_y, int src_w, int src_h, int x, int y, const sf::Color& color)
{
  sf::Sprite sprite;
  sprite.setTexture(*texture);
  sprite.setTextureRect(sf::IntRect{src_x, src_y, src_w, src_h});
  sprite.setColor(color);
  sprite.setPosition(x, y);
  target.draw(sprite);
}

void draw_rectangle(sf::RenderTarget& target, int x, int y, int w, int h, const sf::Color& color)
{
  sf::RectangleShape dotRect;
  dotRect.setSize(sf::Vector2f(w, h));
  dotRect.setFillColor(color);
  dotRect.setPosition(x, y);
  target.draw(dotRect);
}


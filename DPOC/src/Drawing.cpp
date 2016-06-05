#include "Drawing.h"

void draw_texture(sf::RenderTarget& target, const sf::Texture* texture, int x, int y)
{
  sf::Sprite sprite;
  sprite.setTexture(*texture);
  sprite.setPosition(x, y);
  target.draw(sprite);
}

#include "Cache.h"
#include "Picture.h"

Picture::Picture(const std::string& name)
 : m_texture(cache::loadTexture("Resources/Pictures/" + name)),
   m_x(0),
   m_y(0)
{
}

Picture::~Picture()
{
  cache::releaseTexture(m_texture);
}

void Picture::draw(sf::RenderTarget& target) const
{
  if (m_texture)
  {
    sf::Sprite sprite;
    sprite.setTexture(*m_texture);
    sprite.setPosition(m_x, m_y);
    target.draw(sprite);
  }
}

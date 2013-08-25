#ifndef PICTURE_H
#define PICTURE_H

#include <string>

#include <SFML/Graphics.hpp>

class Picture
{
public:
  Picture(const std::string& name);
  ~Picture();

  void setPosition(float x, float y)
  {
    m_x = x;
    m_y = y;
  }

  void draw(sf::RenderTarget& target) const;
private:
  sf::Texture* m_texture;
  float m_x, m_y;
};

#endif

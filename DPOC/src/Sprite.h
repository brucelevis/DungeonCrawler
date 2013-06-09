#ifndef SPRITE_H
#define SPRITE_H

#include <SFML/Graphics.hpp>

#include "Direction.h"

class Sprite
{
public:
  Sprite();
  ~Sprite();
  void create(const std::string& spriteId, int spriteSheetX, int spriteSheetY, int speed);
  void update(Direction direction);
  void render(sf::RenderTarget& target, float x, float y);

  void setFrame(int frame)
  {
    m_frame = frame;
  }

  inline int getWidth() const { return m_width; }
  inline int getHeight() const { return m_height;}
private:
  Sprite(const Sprite&);
  Sprite& operator=(const Sprite&);
private:
  int m_width, m_height;
  int m_frame, m_maxFrame, m_ticksPerFrame, m_ticks;
  Direction m_direction;

  // "Block entry" in sprite sheet.
  int m_spriteSheetX, m_spriteSheetY;

  sf::Sprite m_sprite;
  std::string m_textureName;
};

#endif

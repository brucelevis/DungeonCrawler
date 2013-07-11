#ifndef SPRITE_H
#define SPRITE_H

#include <SFML/Graphics.hpp>

#include "Direction.h"

class Sprite
{
public:
  Sprite();
  virtual ~Sprite();
  void create(const std::string& spriteId, int spriteSheetX, int spriteSheetY, int speed = 10);
  void update(Direction direction);
  virtual void render(sf::RenderTarget& target, float x, float y);

  void setDirection(Direction dir)
  {
    m_direction = dir;
  }

  void setFrame(int frame)
  {
    m_frame = frame;
  }

  inline int getWidth() const { return m_width; }
  inline int getHeight() const { return m_height;}

  std::string getTextureName() const { return m_textureName; }
private:
  Sprite(const Sprite&);
  Sprite& operator=(const Sprite&);
protected:
  int m_width, m_height;
  int m_frame, m_maxFrame, m_ticksPerFrame, m_ticks;
  Direction m_direction;

  // "Block entry" in sprite sheet.
  int m_spriteSheetX, m_spriteSheetY;

  sf::Sprite m_sprite;
  std::string m_textureName;
};

class TileSprite : public Sprite
{
public:
  TileSprite(sf::Texture* tileset, int tileX, int tileY);

  void render(sf::RenderTarget& target, float x, float y);

  void setTileNum(int tileNum);
private:
  int m_tileX, m_tileY;
  sf::Texture* m_tileset;
};

#endif

#include "logger.h"
#include "Cache.h"
#include "Config.h"
#include "Sprite.h"

Sprite::Sprite()
  : m_width(0),
    m_height(0),
    m_frame(0),
    m_maxFrame(config::NUM_SPRITES_X),
    m_ticksPerFrame(0),
    m_ticks(0),
    m_direction(DIR_DOWN),
    m_spriteSheetX(0),
    m_spriteSheetY(0)
{
}

Sprite::~Sprite()
{
  TRACE("Deleting sprite %s", m_textureName.c_str());

  cache::releaseTexture(m_textureName);
}

void Sprite::create(const std::string& spriteId, int spriteSheetX, int spriteSheetY, int speed)
{
  if (!m_textureName.empty())
  {
    TRACE("Recreating sprite %s with new textureName=%s", m_textureName.c_str(), spriteId.c_str());
    cache::releaseTexture(m_textureName);
  }
  else
  {
    TRACE("Creating sprite %s, spriteSheetX=%d, spriteSheety=%d, speed=%d",
        spriteId.c_str(), spriteSheetX, spriteSheetY, speed);
  }

  m_textureName = spriteId;

  m_ticksPerFrame = speed;
  m_spriteSheetX = spriteSheetX;
  m_spriteSheetY = spriteSheetY;

  sf::Texture* spriteTexture = cache::loadTexture(m_textureName);

  if (spriteTexture)
  {
    m_sprite.setTexture(*spriteTexture);

    int numberOfBlocksX = (spriteTexture->getSize().x / (config::TILE_W * config::NUM_SPRITES_X));
    int numberOfBlocksY = (spriteTexture->getSize().y / (config::TILE_H * config::NUM_SPRITES_Y));

    int blockW = spriteTexture->getSize().x / numberOfBlocksX;
    int blockH = spriteTexture->getSize().y / numberOfBlocksY;

    m_width = blockW / config::NUM_SPRITES_X;
    m_height = blockH / config::NUM_SPRITES_Y;
  }
  else
  {
    TRACE("Unable to create sprite %s: Texture not found!", m_textureName.c_str());
  }
}

void Sprite::changeTexture(const std::string& textureName)
{
  create(textureName, m_spriteSheetX, m_spriteSheetY, m_ticksPerFrame);
}

void Sprite::update(Direction direction)
{
  m_direction = direction;
  
  m_ticks++;
  if (m_ticks >= m_ticksPerFrame)
  {
    m_ticks = 0;
    m_frame++;
    if (m_frame >= m_maxFrame)
    {
      m_frame = 0;
    }
  }
}

void Sprite::render(sf::RenderTarget& target, float x, float y)
{
  int rectLeft = m_spriteSheetX * m_width * config::NUM_SPRITES_X + m_frame * m_width;
  int rectTop = m_spriteSheetY * m_height * config::NUM_SPRITES_Y + m_direction * m_height;

  m_sprite.setPosition(x, y);
  m_sprite.setTextureRect(sf::IntRect(rectLeft, rectTop, m_width, m_height));
  target.draw(m_sprite);
}

void Sprite::render_ex(sf::RenderTarget& target, float x, float y, sf::Color color)
{
  m_sprite.setColor(color);

  render(target, x, y);

  m_sprite.setColor(sf::Color::White);
}

Sprite* Sprite::clone() const
{
  Sprite* rhs = new Sprite;
  rhs->create(m_textureName, m_spriteSheetX, m_spriteSheetY);
  rhs->m_frame = m_frame;
  rhs->m_maxFrame = m_maxFrame;
  rhs->m_ticksPerFrame = m_ticksPerFrame;
  rhs->m_ticks = m_ticks;
  rhs->m_direction = m_direction;
  return rhs;
}

///////////////////////////////////////////////////////////////////////////////

TileSprite::TileSprite(sf::Texture* tileset, int tileX, int tileY)
 : m_tileX(tileX),
   m_tileY(tileY),
   m_tileset(tileset)
{
  TRACE("Creating new TileSprite. tileX=%d, tileY=%d", tileX, tileY);

  m_sprite.setTexture(*tileset);
  m_sprite.setTextureRect(sf::IntRect(tileX * config::TILE_W, tileY * config::TILE_H, config::TILE_W, config::TILE_H));
}

void TileSprite::render(sf::RenderTarget& target, float x, float y)
{
  m_sprite.setPosition(x, y);
  target.draw(m_sprite);
}

void TileSprite::setTileNum(int tileNum)
{
  m_tileX = tileNum % (m_tileset->getSize().x / config::TILE_W);
  m_tileY = tileNum / (m_tileset->getSize().x / config::TILE_H);

  m_sprite.setTextureRect(sf::IntRect(m_tileX * config::TILE_W, m_tileY * config::TILE_H, config::TILE_W, config::TILE_H));
}

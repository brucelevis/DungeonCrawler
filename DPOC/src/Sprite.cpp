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
    m_direction(0),
    m_spriteSheetX(0),
    m_spriteSheetY(0)
{
}

Sprite::~Sprite()
{
  cache::releaseTexture(m_textureName);
}

void Sprite::create(const std::string& spriteId, int spriteSheetX, int spriteSheetY, int speed)
{
  if (!m_textureName.empty())
  {
    TRACE("Recreating sprite %s with new textureName=%s", m_textureName.c_str(), spriteId.c_str());
    cache::releaseTexture(m_textureName);
  }

  m_textureName = spriteId;

  m_ticksPerFrame = speed;
  m_spriteSheetX = spriteSheetX;
  m_spriteSheetY = spriteSheetY;

  sf::Texture* spriteTexture = cache::loadTexture(m_textureName);

  if (spriteTexture)
  {
    m_sprite.setTexture(*spriteTexture);
    m_width = (spriteTexture->getSize().x / (config::TILE_W * config::NUM_SPRITES_X)) / config::NUM_SPRITES_X;
    m_height = (spriteTexture->getSize().y / (config::TILE_H * config::NUM_SPRITES_Y)) / config::NUM_SPRITES_Y;
  }
  else
  {
    TRACE("Unable to create sprite %s: Texture not found!", m_textureName.c_str());
  }
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
  int rectLeft = m_spriteSheetX * m_width + m_frame * m_width;
  int rectTop = m_spriteSheetY * m_height + m_direction * m_height;

  m_sprite.setPosition(x, y);
  m_sprite.setTextureRect(sf::IntRect(rectLeft, rectTop, m_width, m_height));
  target.draw(m_sprite);
}

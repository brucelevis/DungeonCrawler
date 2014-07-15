#include "logger.h"
#include "Config.h"

#include "Cache.h"
#include "Sound.h"
#include "Effect.h"

static const std::map<std::string, EffectDef> frames =
{
  {
    "Effect_Flame",
    {
      "Animations/effect_Flame.png",
      16,
      {
        { 0, 6, 0, 0, "Audio/boom.wav", 1, 0, sf::Color::White },
        { 1, 6, 0, 0, "", 1.5, 0, sf::Color::White },
        { 2, 6, 0, 0, "", 2.0, 0, {255, 255, 255, 200 } },
        { 0, 6, 0, 0, "", 2.5, 0, {255, 255, 255, 150 } },
        { 1, 6, 0, 0, "", 3.0, 0, {255, 255, 255, 100 } },
        { 2, 6, 0, 0, "", 3.0, 0, {255, 255, 255, 50 } }
      }
    }
  },
  {
    "Effect_Slash",
    {
      "Animations/effect_Slash.png",
      24,
      {
        { 0, 6, 0, 0, "Audio/Sword1.wav", 1, 0, sf::Color::White },
        { 1, 4, 0, 0, "", 1, 0, sf::Color::White },
        { 2, 3, 0, 0, "", 1, 0, sf::Color::White },
        { 3, 4, 0, 0, "", 1, 0, sf::Color::White },
        { 4, 6, 0, 0, "", 1, 0, sf::Color::White }
      }
    }
  },
  {
    "Effect_Hit",
    {
      "Animations/effect_Hit.png",
      64,
      {
        { 0, 6, 0, 0, "Audio/Blow2.wav", 1, 0, sf::Color::White },
        { 1, 6, 0, 0, "", 1, 0, sf::Color::White },
        { 2, 6, 0, 0, "", 1, 0, sf::Color::White }
      }
    }
  }
};

Effect::Effect()
  : m_originX(0),
    m_originY(0),
    m_spriteSize(0),
    m_texture(0),
    m_currentFrameIndex(0),
    m_currentTime(0)
{

}

Effect::~Effect()
{
  cache::releaseTexture(m_texture);
}

void Effect::loadTexture(const std::string& texture, int spriteSize)
{
  m_texture = cache::loadTexture(texture);
  m_spriteSize = spriteSize;
  m_drawSprite.setTexture(*m_texture);
}

void Effect::update()
{
  if (complete())
  {
    return;
  }

  if (m_currentTime == 0)
  {
    std::string sound = getCurrentFrame()->sound;
    if (!sound.empty())
    {
      play_sound(sound);
    }
  }

  m_currentTime++;

  Frame* currentFrame = getCurrentFrame();
  if (m_currentTime >= currentFrame->time)
  {
    m_currentTime = 0;
    m_currentFrameIndex++;

    if (!complete())
    {
      initSprite();
    }
  }
}

void Effect::setOrigin(float x, float y)
{
  m_originX = x;
  m_originY = y;

  initSprite();
}

void Effect::render(sf::RenderTarget& target)
{
  target.draw(m_drawSprite);
}

Effect::Frame* Effect::getCurrentFrame()
{
  return &m_frames[m_currentFrameIndex];
}

void Effect::initSprite()
{
  if (!m_texture)
  {
    TRACE("Effect::initSprite: Texture not loaded!\n");
    return;
  }

  Frame* currentFrame = getCurrentFrame();

  int sprX = m_spriteSize * (currentFrame->index % (m_texture->getSize().x / m_spriteSize));
  int sprY = m_spriteSize * (currentFrame->index / (m_texture->getSize().x / m_spriteSize));

  m_drawSprite.setTextureRect(sf::IntRect(sprX, sprY, m_spriteSize, m_spriteSize));
  m_drawSprite.setColor(currentFrame->blendColor);
  m_drawSprite.setScale(currentFrame->scale, currentFrame->scale);
  m_drawSprite.setRotation(currentFrame->rotate);

  float origX = m_originX - (m_spriteSize * (currentFrame->scale - 1)) / 2;
  float origY = m_originY - (m_spriteSize * (currentFrame->scale - 1)) / 2;

  m_drawSprite.setPosition(origX + currentFrame->displaceX, origY + currentFrame->displaceY);
}

Effect* Effect::createEffect(const std::string& effectName, int x, int y)
{
  auto it = frames.find(effectName);

  if (it != frames.end())
  {
    Effect* effect = new Effect;
    effect->loadTexture(it->second.texture, it->second.frameSize);
    for (auto frameIt = it->second.frames.begin(); frameIt != it->second.frames.end(); ++frameIt)
    {
      effect->m_frames.push_back(*frameIt);
    }
    effect->initSprite();
    effect->setOrigin(x, y);

    return effect;
  }

  return 0;
}

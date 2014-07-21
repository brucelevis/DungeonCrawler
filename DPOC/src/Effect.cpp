#include <stdexcept>
#include <sstream>

#include "logger.h"
#include "Config.h"

#include "Cache.h"
#include "Sound.h"
#include "Effect.h"

#include "../dep/tinyxml2.h"
using namespace tinyxml2;

template <typename T>
T parseAttribute(const XMLElement* element, const std::string& attrName)
{
  const XMLAttribute* attribute = element->FindAttribute(attrName.c_str());

  if (attribute)
  {
    T t;

    std::istringstream ss ( attribute->Value() );
    ss >> t;

    return t;
  }
  else
  {
    throw std::runtime_error("No attribute: " + attrName);
  }

  return T();
}

void BattleAnimation::load(const std::string& file)
{
  XMLDocument doc;
  if (doc.LoadFile(file.c_str()) != 0)
  {
    fprintf(stderr, "Couldn't find %s!\n", file.c_str());
    exit(1);
  }

  const XMLElement* root = doc.FirstChildElement("animation");

  m_texture = cache::loadTexture("Animations/" + parseAttribute<std::string>(root, "texture"));
  m_spriteWidth = parseAttribute<int>(root, "textureW");
  m_spriteHeight = parseAttribute<int>(root, "textureH");

  for (const XMLElement* element = root->FirstChildElement(); element; element = element->NextSiblingElement())
  {
    std::string name = element->Name();
    if (name == "frame")
    {
      Frame frame;
      frame.sprites = parseFrame(element);
      m_frames.push_back(frame);
    }
  }
}

std::vector<BattleAnimation::Frame::Sprite> BattleAnimation::parseFrame(const tinyxml2::XMLElement* frameElement)
{
  std::vector<Frame::Sprite> sprites;

  for (const XMLElement* element = frameElement->FirstChildElement(); element; element = element->NextSiblingElement())
  {
    std::string name = element->Name();
    if (name == "sprite")
    {
      int index = parseAttribute<int>(element, "index");
      int x     = parseAttribute<int>(element, "x");
      int y     = parseAttribute<int>(element, "y");
      float scaleX = parseAttribute<float>(element, "scaleX");
      float scaleY = parseAttribute<float>(element, "scaleY");
      int alpha = parseAttribute<int>(element, "alpha");

      Frame::Sprite sprite;
      sprite.index = index;
      sprite.x = x;
      sprite.y = y;
      sprite.scaleX = scaleX;
      sprite.scaleY = scaleY;
      sprite.alpha = alpha;

      sprites.push_back(sprite);
    }
  }

  return sprites;
}

BattleAnimation::BattleAnimation()
  : m_originX(0),
    m_originY(0),
    m_spriteWidth(0),
    m_spriteHeight(0),
    m_texture(0),
    m_currentFrameIndex(0)
{

}

BattleAnimation::~BattleAnimation()
{
  cache::releaseTexture(m_texture);
}

void BattleAnimation::update()
{
  if (complete())
  {
    return;
  }
}

void BattleAnimation::setOrigin(float x, float y)
{
  m_originX = x;
  m_originY = y;
}

void BattleAnimation::render(sf::RenderTarget& target)
{
  if (complete())
    return;

  Frame* frame = getCurrentFrame();
  for (auto it = frame->sprites.begin(); it != frame->sprites.end(); ++it)
  {
    sf::Sprite sprite;
    sprite.setTexture(*m_texture);

    int textureX = m_spriteWidth * (it->index % (m_texture->getSize().x / m_spriteWidth));
    int textureY = m_spriteHeight * (it->index / (m_texture->getSize().x / m_spriteWidth));

    sprite.setTextureRect(sf::IntRect(textureX, textureY, m_spriteWidth, m_spriteHeight));
    sprite.setOrigin(m_spriteWidth / 2, m_spriteHeight / 2);
    sprite.setScale(it->scaleX, it->scaleY);

    sf::Color color = sprite.getColor();
    color.a = it->alpha;
    sprite.setColor(color);

    sprite.setPosition(m_originX + it->x, m_originY + it->y);

    target.draw(sprite);
  }

  m_currentFrameIndex++;
}

BattleAnimation::Frame* BattleAnimation::getCurrentFrame()
{
  return &m_frames[m_currentFrameIndex];
}

BattleAnimation* BattleAnimation::loadEffect(const std::string& filename)
{
  BattleAnimation* effect = new BattleAnimation;
  effect->load(filename);
  return effect;
}

#ifndef EFFECT_H
#define EFFECT_H

#include <SFML/Graphics.hpp>

#include <vector>
#include <string>

namespace tinyxml2
{
  class XMLElement;
}

class Effect
{
public:
  struct Frame
  {
    struct Sprite
    {
      int index;
      int x, y;
      float scaleX, scaleY;
      int alpha;
    };
    std::vector<Sprite> sprites;
  };

  Effect();
  ~Effect();

  void update();

  void setOrigin(float x, float y);

  void render(sf::RenderTarget& target);

  inline bool complete() const
  {
    return m_currentFrameIndex >= m_frames.size();
  }

  static Effect* loadEffect(const std::string& filename);
private:
  Frame* getCurrentFrame();

  void initSprite();

  void load(const std::string& filename);
  std::vector<Frame::Sprite> parseFrame(const tinyxml2::XMLElement* frameElement);
private:
  float m_originX, m_originY;

  int m_spriteWidth;
  int m_spriteHeight;

  sf::Texture* m_texture;

  std::vector<Frame> m_frames;
  size_t m_currentFrameIndex;
};

#endif

#ifndef EFFECT_H
#define EFFECT_H

#include <SFML/Graphics.hpp>

#include <vector>
#include <string>

class Effect
{
public:
  struct Frame
  {
    int index;
    int time;
    float displaceX, displaceY;
    std::string sound;

    float scale;
    float rotate;
    sf::Color blendColor;
  };

  Effect();
  ~Effect();

  void loadTexture(const std::string& texture, int spriteSize);

  void update();

  void setOrigin(float x, float y);

  void render(sf::RenderTarget& target);

  inline bool complete() const
  {
    return m_currentFrameIndex >= m_frames.size();
  }

  static Effect* createEffect(const std::string& effectName, int x, int y);
private:
  Frame* getCurrentFrame();

  void initSprite();
private:
  float m_originX, m_originY;
  int m_spriteSize;

  sf::Texture* m_texture;
  sf::Sprite m_drawSprite;

  std::vector<Frame> m_frames;
  size_t m_currentFrameIndex;
  int m_currentTime;
};

struct EffectDef
{
  std::string texture;
  int frameSize;

  std::vector<Effect::Frame> frames;
};

#endif

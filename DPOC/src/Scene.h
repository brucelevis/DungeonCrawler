#ifndef SCENE_H
#define SCENE_H

#include <SFML/Graphics.hpp>

class Scene
{
public:
  enum FadeType
  {
    FADE_NONE,
    FADE_IN,
    FADE_OUT
  };

  Scene() : m_done(false) {}

  virtual ~Scene() {}

  virtual void update() = 0;
  virtual void draw(sf::RenderTexture& target) = 0;
  virtual void handleEvent(sf::Event& event) = 0;

  virtual void preFade(FadeType) {}
  virtual void postFade(FadeType) {}

  void close() { m_done = true; }
  bool isClosed() const { return m_done; }
private:
  bool m_done;
};

#endif

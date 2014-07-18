#ifndef CHARGEN_H
#define CHARGEN_H

#include "Scene.h"

class CharGen : public Scene
{
public:
  void update();
  void draw(sf::RenderTarget& target);
  void handleEvent(sf::Event& event);
private:
  void handleKeyPress(sf::Keyboard::Key key);
private:
};

#endif

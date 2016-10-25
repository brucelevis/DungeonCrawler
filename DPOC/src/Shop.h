#ifndef SHOP_H
#define SHOP_H

#include <vector>
#include <string>

#include <SFML/Graphics.hpp>

#include "Scene.h"

class PlayerCharacter;

class Shop : public Scene
{
public:
  Shop(const std::vector<std::string>& items);

  void update();
  void draw(sf::RenderTarget& target);
  void handleEvent(sf::Event& event);
private:
  void menuClosed();
  void handleKeyPress(sf::Keyboard::Key key);
};

#endif

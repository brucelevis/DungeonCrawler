#ifndef CHARGEN_H
#define CHARGEN_H

#include "Scene.h"

class Menu;
class Player;

class CharGen : public Scene
{
public:
  CharGen();
  ~CharGen();

  void update();
  void draw(sf::RenderTarget& target);
  void handleEvent(sf::Event& event);
private:
  void handleKeyPress(sf::Keyboard::Key key);
private:
  Player* m_player;
  Menu* m_selectMenu;
};

#endif

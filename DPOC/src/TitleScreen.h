#ifndef TITLE_SCREEN_H
#define TITLE_SCREEN_H

#include <SFML/Audio.hpp>
#include <SFML/Graphics.hpp>

#include "Direction.h"
#include "Scene.h"
#include "Menu_TitleMenu.h"

class TitleScreen : public Scene
{
public:
  TitleScreen();
  ~TitleScreen();

  void update();
  void draw(sf::RenderTarget& target);
  void handleEvent(sf::Event& event);

  void postFade(FadeType fadeType);
private:
  void handleTitleOption(TitleMenu::Action action);

  void handleKeyPress(sf::Keyboard::Key key);
private:
  sf::Texture* m_titleTexture;
  sf::Music m_titleMusic;
  TitleMenu::Action m_selectedAction;
};

#endif

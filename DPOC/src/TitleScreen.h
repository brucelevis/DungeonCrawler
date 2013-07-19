#ifndef TITLE_SCREEN_H
#define TITLE_SCREEN_H

#include <SFML/Audio.hpp>
#include <SFML/Graphics.hpp>

#include "Scene.h"
#include "Menu.h"

class TitleMenu : public Menu
{
public:
  void handleConfirm();
private:
};

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
  void handleKeyPress(sf::Keyboard::Key key);
private:
  TitleMenu m_menu;
  sf::Texture* m_titleTexture;
  sf::Music m_titleMusic;
};

#endif

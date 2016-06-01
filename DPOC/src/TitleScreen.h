#ifndef TITLE_SCREEN_H
#define TITLE_SCREEN_H

#include <SFML/Audio.hpp>
#include <SFML/Graphics.hpp>

#include "Direction.h"
#include "Scene.h"
#include "Menu.h"
#include "SaveMenu.h"

class TitleMenu : public Menu
{
public:
  TitleMenu();
  ~TitleMenu();

  void handleConfirm();
  void moveArrow(Direction dir);
  void handleEscape();

  void draw(sf::RenderTarget& target, int x, int y);
private:
  SaveMenu* m_saveMenu;
};

class TitleScreen : public Scene
{
public:
  TitleScreen();
  ~TitleScreen();

  void update();
  void draw(sf::RenderTexture& target);
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

#ifndef BATTLEBACKGROUND_H_
#define BATTLEBACKGROUND_H_

#include <SFML/Graphics.hpp>

#include "Direction.h"
#include "Camera.h"

class Raycaster;
class Map;

/**
 * Using raycaster it will attempt to draw a battle background that matches
 * what the player can see somewhat.
 */

class BattleBackground
{
public:
  BattleBackground(Map* sourceMap, int x, int y, Direction playerDir);
  ~BattleBackground();

  const sf::Texture& getBackgroundTexture();
private:
  sf::Image m_raycasterBuffer;
  sf::Texture m_raycasterTexture;

  Raycaster* m_raycaster;
  Map* m_currentMap;

  Camera m_camera;
};

#endif

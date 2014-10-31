#ifndef MINIMAP_H
#define MINIMAP_H

#include <SFML/Graphics.hpp>

class Map;

class Minimap
{
public:
  Minimap(int x, int y, int w, int h);
   ~Minimap();

  void updatePosition(Map* currentMap, int x, int y, int playerX, int playerY);

  void draw(sf::RenderTarget& target) const;
private:
  int m_x, m_y;
  int m_w, m_h;
  int m_centerX, m_centerY;
  int m_playerX, m_playerY;
  Map* m_currentMap;

  sf::Texture* m_campsiteIcon;
  sf::Texture* m_doorIcon;
};

#endif

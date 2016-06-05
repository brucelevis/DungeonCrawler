#ifndef MINIMAP_H
#define MINIMAP_H

#include <SFML/Graphics.hpp>
#include "Direction.h"

class Map;

class Minimap
{
public:
  Minimap(int x, int y, int w, int h);
   ~Minimap();

  void updatePosition(Map* currentMap, int x, int y, int playerX, int playerY);

  void draw(sf::RenderTarget& target) const;
private:
  void drawArrow(sf::RenderTarget& target, Direction direction, int tx, int ty) const;
  void drawTile(sf::RenderTarget& target, int tileId, int tx, int ty, const sf::Color& color = sf::Color::White) const;
private:
  int m_x, m_y;
  int m_w, m_h;
  int m_centerX, m_centerY;
  int m_playerX, m_playerY;
  Map* m_currentMap;

  sf::Texture* m_mapTiles;
  sf::Texture* m_arrowTexture;
};

#endif

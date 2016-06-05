#ifndef DUNGEONMAP_H_
#define DUNGEONMAP_H_

#include <string>

#include "Scene.h"
#include "Minimap.h"

class Map;

class DungeonMap : public Scene
{
public:
  DungeonMap(Map* map);
  ~DungeonMap();

  void update();

  void draw(sf::RenderTarget& target);

  void handleEvent(sf::Event& event);
private:
  Map* m_map;
  Minimap m_minimap;
  int m_centerX, m_centerY;
  sf::Texture* m_arrowTexture;
  sf::Texture* m_mapTexture;
};


#endif /* DUNGEONMAP_H_ */

#ifndef RAYCASTER_H
#define RAYCASTER_H

#include <list>

#include <SFML/Graphics.hpp>

#include "Vec2.h"
#include "Camera.h"
#include "Direction.h"

class Entity;
class Map;

class Raycaster
{
public:
  Raycaster(int width, int height);

  void raycast(Camera* camera, sf::Image& buffer, bool wireframe = false, Direction pDir = DIR_RANDOM);
  
  void setTilemap(Map* tilemap);
  
  void addEntity(const Entity* entity);
  void removeEntity(const Entity* entity);
 
  void clearEntities() { m_entities.clear(); }

private:
  struct RayInfo {
    float wallDist;
    int mapX, mapY;
    float floorXWall, floorYWall;
    int textureX;
    int side;
  };

  RayInfo castRay(int x, int width, int height);

  bool sameCoord(const RayInfo& a, const RayInfo& b) const;

  void drawFloorsCeiling(const RayInfo& info, int x, int wallEnd, sf::Image& buffer, bool wireframe);
  void drawSprites(sf::Image& buffer, Direction pDir, bool wireframe);

  const Entity* getEntityAt(int x, int y) const;

  void drawWallFeature(sf::Image& buffer, bool isWireframe, sf::Image& image, int textureX, int textureY, int wallDist, int x, int y);
private:
  int m_width, m_height;

  Camera* m_camera;
  Map* m_tilemap;

  std::vector<sf::Image> m_tileTextures;

  std::list<const Entity*> m_entities;

};

#endif

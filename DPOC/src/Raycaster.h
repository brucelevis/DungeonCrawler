#ifndef RAYCASTER_H
#define RAYCASTER_H

#include <list>

#include <SFML/Graphics.hpp>

#include "Vec2.h"
#include "Camera.h"
#include "Direction.h"

class Entity;
class Map;
class Tile;

class Raycaster
{
public:
  Raycaster(int width, int height);

  void raycast(Camera* camera, sf::Image& buffer, Direction pDir = DIR_RANDOM);
  
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

  void drawWallSlice(const RayInfo& info, sf::Image& buffer, int x, int lineHeight, int wallStart, int wallEnd, const std::string& layer);

  RayInfo castRay(int x, int width) const;
  RayInfo castDoorRay(int x, int width) const;

  bool sameCoord(const RayInfo& a, const RayInfo& b) const;

  void drawFloorsCeiling(const RayInfo& info, int x, int wallEnd, sf::Image& buffer);
  void drawSprites(sf::Image& buffer, Direction pDir);

  const Entity* getEntityAt(int x, int y) const;

  bool outOfBounds(int mapX, int mapY) const;
private:
  int m_width, m_height;

  Camera* m_camera;
  Map* m_tilemap;

  std::vector<sf::Image> m_tileTextures;

  std::list<const Entity*> m_entities;

};

#endif

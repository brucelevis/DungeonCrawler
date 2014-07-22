#include <cmath>
#include <cstdlib>

#include <vector>
#include <algorithm>

#include <SFML/Graphics.hpp>

#include "Config.h"

#include "Vec2.h"
#include "Map.h"
#include "Entity.h"
#include "Sprite.h"

#include "Raycaster.h"

float zbuffer[320];

namespace
{
  const float INTENSITY = 0.75f;
  const float MULTIPLIER = 2.0f;
}

sf::Color computeIntensity(const sf::Color& pixel, float objectIntensity, float multiplier, float distance) 
{
    float intensity = objectIntensity / distance * multiplier;

    sf::Uint8 orig_r = pixel.r;
    sf::Uint8 orig_g = pixel.g;
    sf::Uint8 orig_b = pixel.b;

    float fr = (float) pixel.r;
    float fg = (float) pixel.g;
    float fb = (float) pixel.b;

    fr *= intensity;
    fg *= intensity;
    fb *= intensity;

    if (fr > orig_r) fr = orig_r;
    if (fg > orig_g) fg = orig_g;
    if (fb > orig_b) fb = orig_b;

    return sf::Color((sf::Uint8) fr, (sf::Uint8) fg, (sf::Uint8) fb);
}

Raycaster::Raycaster(int width, int height)
 : m_width(width),
   m_height(height),
   m_camera(0),
   m_tilemap(0)
{
}

void Raycaster::setTilemap(Map* tilemap)
{
  m_tilemap = tilemap;
  m_tileTextures = tilemap->getTilesetImages();
}

void Raycaster::addEntity(const Entity* entity)
{
  m_entities.push_back(entity);
}

void Raycaster::removeEntity(const Entity* entity)
{
  m_entities.remove(entity);
}

void Raycaster::raycast(Camera* camera, sf::Image& buffer, Direction pDir)
{
  m_camera = camera;

  int x, y;

  for (x = 0; x < m_width; x++)
  {
    int lineHeight;
    int wallStart, wallEnd;
    
    RayInfo info = castRay(x, m_width, m_height);

    zbuffer[x] = info.wallDist;
    
    lineHeight = (int)fabs((float)m_height / info.wallDist);
    wallStart = -lineHeight / 2 + m_height / 2;
    wallEnd = lineHeight / 2 + m_height / 2;
    
    if  (wallStart < 0) wallStart = 0;
    
    for (y = wallStart; y < std::min(wallEnd, m_height); y++) 
    {
      int d, textureY;

      d = y * 256 - m_height * 128 + lineHeight * 128;
      textureY = ((d * config::TILE_W) / lineHeight) / 256;

      Tile* tile = m_tilemap->getTileAt(info.mapX, info.mapY, "wall");
      if (tile)
      {
        int tileId = tile ? tile->tileId : -1;

        sf::Color color = computeIntensity(
            m_tileTextures[tileId].getPixel(info.textureX, textureY),
            INTENSITY, MULTIPLIER, info.wallDist);

        buffer.setPixel(x, y, color);

        Tile* featureTile = m_tilemap->getTileAt(info.mapX, info.mapY, "wallfeature");
        if (featureTile && featureTile->tileId > -1)
        {
          color = m_tileTextures[featureTile->tileId].getPixel(info.textureX, textureY);

          if (color.a == 255)
          {
            color = computeIntensity(
                color,
                INTENSITY, MULTIPLIER, info.wallDist);

            buffer.setPixel(x, y, color);
          }
        }
      }
      else
      {
        buffer.setPixel(x, y, sf::Color::Black);
      }
    }
  
    if (wallEnd < 0) 
    {
      wallEnd = m_height;
    }

    drawFloorsCeiling(info, x, wallEnd, buffer);
  }

  drawSprites(buffer, pDir);
}

void Raycaster::drawFloorsCeiling(const RayInfo& info, int x, int wallEnd, sf::Image& buffer)
{
  float cameraDist = 0;

  for (int y = wallEnd; y < m_height; y++) 
  {
    int floorTextureX, floorTextureY;
    float weight;
    float currentFloorX, currentFloorY;
    int textureIndex;

    float currentDist = (float)m_height / (2.0f * y - m_height);
      
    weight = (currentDist - cameraDist) / (info.wallDist - cameraDist);
    currentFloorX = weight * info.floorXWall + (1.0f - weight) * m_camera->pos.x;
    currentFloorY = weight * info.floorYWall + (1.0f - weight) * m_camera->pos.y;
    floorTextureX = (int)(currentFloorX * config::TILE_W) % config::TILE_W;
    floorTextureY = (int)(currentFloorY * config::TILE_H) % config::TILE_H;
      
    textureIndex = (int)currentFloorY * m_tilemap->getWidth() + (int)currentFloorX;

    Tile* floorTile = m_tilemap->getTileAt((int) currentFloorX, (int) currentFloorY, "floor");
    Tile* ceilTile = m_tilemap->getTileAt((int) currentFloorX, (int) currentFloorY, "ceiling");

    int floorIndex = floorTile ? floorTile->tileId : -1;
    int ceilIndex = ceilTile ? ceilTile->tileId : -1;

    // Floor
    if (floorIndex > -1)
    {
      sf::Color color = computeIntensity(
          m_tileTextures[floorIndex].getPixel(floorTextureX, floorTextureY),
          0.75, 1.0, currentDist);
      buffer.setPixel(x, y, color);
    }

    // Ceiling
    if (ceilIndex > -1)
    {
      sf::Color color = computeIntensity(
          m_tileTextures[ceilIndex].getPixel(floorTextureX, floorTextureY),
          INTENSITY, MULTIPLIER, currentDist);
      buffer.setPixel(x, m_height - y, color);
    }
  }
}

void Raycaster::drawSprites(sf::Image& buffer, Direction pDir)
{
  m_entities.sort([=](const Entity* lhs, const Entity* rhs) -> bool
    {
      float leftDistance = (m_camera->pos.x - lhs->x) * (m_camera->pos.x - lhs->x) +
                           (m_camera->pos.y - lhs->y) * (m_camera->pos.y - lhs->y);
      float rightDistance = (m_camera->pos.x - rhs->x) * (m_camera->pos.x - rhs->x) +
                            (m_camera->pos.y - rhs->y) * (m_camera->pos.y - rhs->y);
  
      return leftDistance > rightDistance;
    });

  for (auto sprIter = m_entities.begin(); sprIter != m_entities.end(); ++sprIter)
  {
    if (!(*sprIter)->isVisible() || (*sprIter)->sprite() == 0)
    {
      continue;
    }

    float spriteX = 0.5f + ((*sprIter)->x - m_camera->pos.x);
    float spriteY = 0.5f + ((*sprIter)->y - m_camera->pos.y);

    float invDet = 1.0f / (m_camera->plane.x * m_camera->dir.y - m_camera->dir.x * m_camera->plane.y);

    float transformX = invDet * (m_camera->dir.y * spriteX - m_camera->dir.x * spriteY);
    float transformY = invDet * (-m_camera->plane.y * spriteX + m_camera->plane.x * spriteY);

    int spriteScreenX = static_cast<int>(
        static_cast<float>((m_width / 2)) * (1.0f + transformX / transformY)
      );

    // Height of sprite
    int spriteHeight = abs( static_cast<int> ( static_cast<float>(m_height) / transformY) );

    int drawStartY = -spriteHeight / 2 + m_height / 2;
    if (drawStartY < 0) drawStartY = 0;

    int drawEndY = spriteHeight / 2 + m_height / 2;
    if (drawEndY >= m_height) drawEndY = m_height;

    // Width of sprite
    int spriteWidth = abs( static_cast<int> ( static_cast<float>(m_height) / transformY) );

    int drawStartX = -spriteWidth / 2 + spriteScreenX;
    if (drawStartX < 0) drawStartX = 0;

    int drawEndX = spriteWidth / 2 + spriteScreenX;
    if (drawEndX < 0) drawEndX = 0;
    if (drawEndX >= m_width) drawEndX = m_width;

    if (drawStartX < drawEndX)
    {
      sf::Image spriteImage = (*sprIter)->sprite()->getImage(pDir);
      
      for (int x = drawStartX; x < drawEndX; x++)
      {
        int texX = static_cast<int>(
          256 * (x - (-spriteWidth / 2 + spriteScreenX)) * (*sprIter)->sprite()->getWidth() /*config::TILE_SIZE*/ / spriteWidth
        ) / 256;

        if (transformY > 0 && x >= 0 && x < m_width && transformY < zbuffer[x])
        {
          for (int y = drawStartY; y < drawEndY; y++)
          {
            int d = y * 256 - m_height * 128 + spriteHeight * 128;
            int texY = ((d * (*sprIter)->sprite()->getHeight() /*config::TILE_SIZE*/) / spriteHeight) / 256;

            sf::Color color = spriteImage.getPixel(texX, texY);

            if (color.a == 255)
            {
              color = computeIntensity(color, INTENSITY, MULTIPLIER, transformY);
              buffer.setPixel(x, y, color);
            }
          }
        }

      }
    }

  }
}

Raycaster::RayInfo Raycaster::castRay(int x, int width, int height) 
{
  int mapX, mapY;
  float sideDistX, sideDistY;
  float ddx, ddy;
  float wallDist;
  int stepX, stepY;
  int side;
  float wallX;
  int textureX;
  float floorXWall, floorYWall;
  
  float camX = 2.0f * (float)x / (float)width - 1.0f;
  Vec2 ray = m_camera->pos;
  Vec2 rayDir(m_camera->dir.x + m_camera->plane.x * camX,
              m_camera->dir.y + m_camera->plane.y * camX);
  
  mapX = (int) ray.x;
  mapY = (int) ray.y;
  
  ddx = sqrt(1.0f + (rayDir.y * rayDir.y) / (rayDir.x * rayDir.x));
  ddy = sqrt(1.0f + (rayDir.x * rayDir.x) / (rayDir.y * rayDir.y));
  
  if (rayDir.x < 0)
  {
    stepX = -1;
    sideDistX = (ray.x - mapX) * ddx;
  }
  else
  {
    stepX = 1;
    sideDistX = (mapX + 1.0f - ray.x) * ddx; 
  }
  
  if (rayDir.y < 0)
  {
    stepY = -1;
    sideDistY = (ray.y - mapY) * ddy;
  } else
  {
    stepY = 1;
    sideDistY = (mapY + 1.0f - ray.y) * ddy;
  }
  
  while (1)
  {
    if (sideDistX < sideDistY)
    {
      sideDistX += ddx;
      mapX += stepX;
      side = 0;
    } 
    else
    {
      sideDistY += ddy;
      mapY += stepY;
      side = 1;
    }
    
    if ((mapX < 0 || mapY < 0 || mapX >= m_tilemap->getWidth() || mapY >= m_tilemap->getHeight()) || 
        /*m_tilemap->blocking(mapX, mapY)*/m_tilemap->getTileAt(mapX, mapY, "wall")->tileId != -1)
    {
      break;
    }
  }
  
  if (side == 0)
  {
    wallDist = fabs((mapX - ray.x + (1.0f - stepX) / 2.0f) / rayDir.x);
    wallX = ray.y + ((mapX - ray.x + (1.0f - stepX) / 2.0f) / rayDir.x) * rayDir.y;
    wallX -= floor(wallX);
    
    textureX = (int)(wallX * (float)config::TILE_W);
    if (rayDir.x > 0)
    {
      textureX = config::TILE_W - textureX - 1;
    }
  }
  else
  {
    wallDist = fabs((mapY - ray.y + (1.0f - stepY) / 2.0f) / rayDir.y);
    wallX = ray.x + ((mapY - ray.y + (1.0f - stepY) / 2.0f) / rayDir.y) * rayDir.x;
    wallX -= floor(wallX);
    
    textureX = (int)(wallX * (float)config::TILE_W);
    if (rayDir.y < 0)
    {
      textureX = config::TILE_W - textureX - 1;
    }      
  }
  
  if (side == 0 && rayDir.x > 0)
  {
    floorXWall = (float) mapX;
    floorYWall = mapY + wallX;
  }
  else if (side == 0 && rayDir.x < 0)
  {
    floorXWall = mapX + 1.0f;
    floorYWall = mapY + wallX;
  }
  else if (side == 1 && rayDir.y > 0)
  {
    floorXWall = mapX + wallX;
    floorYWall = (float) mapY;
  }
  else
  {
    floorXWall = mapX + wallX;
    floorYWall = mapY + 1.0f;
  }
  
  RayInfo info =
  {
    wallDist,
    mapX,
    mapY,
    floorXWall,
    floorYWall,
    textureX,
    side
  };
  
  return info;
}

bool Raycaster::sameCoord(const RayInfo& a, const RayInfo& b) const
{
  return a.mapX == b.mapX && a.mapY == b.mapY;
}

const Entity* Raycaster::getEntityAt(int x, int y) const
{
  for (auto it = m_entities.begin(); it != m_entities.end(); ++it)
  {
    int px = (*it)->x;
    int py = (*it)->y;

    if (px == x && py == y)
    {
      return *it;
    }
  }

  return 0;
}

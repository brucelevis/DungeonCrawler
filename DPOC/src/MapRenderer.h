#ifndef MAPRENDERER_H_
#define MAPRENDERER_H_

#include <vector>

#include <SFML/Graphics.hpp>

#include <GameLib/Model.h>
#include <GameLib/Shader.h>
#include <GameLib/Camera.h>

#include <glm/glm.hpp>

#include "Map.h"

class MapRenderer
{
public:
  MapRenderer(Map* tilemap);

  void render(sf::RenderTarget& target, const glm::mat4& projectionMatrix, const gamelib::Camera& camera) const;
private:
  void initMeshes();
private:
  Map* m_tilemap;

  gamelib::Shader m_lightShader;
  gamelib::Model  m_wallCube;

  std::vector<sf::Texture> m_textures;
};

#endif /* MAPRENDERER_H_ */

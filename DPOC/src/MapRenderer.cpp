#include <vector>

#include <GL/glew.h>
#include <glm/glm.hpp>
#include <GameLib/Material.h>

#include "Shaders.h"

#include "MapRenderer.h"

namespace
{
  glm::vec3 _get_normal(const glm::vec3& v1, const glm::vec3& v2, const glm::vec3& v3)
  {
    return glm::normalize(glm::cross((v2 - v1), (v3-v1)));
  }

  void _draw_mesh(const glm::mat4& mvp, const glm::mat4& modelMatrix, const glm::mat4& viewMatrix, const glm::vec3& lightPosition,
    const gamelib::Material& material,
    const gamelib::Model& model,
    const gamelib::Shader& shader,
    const sf::Texture& texture)
  {
    shader.apply();

    unsigned int mvpLoc = glGetUniformLocation(shader, "MVP");

    unsigned int modelMatrixLoc = glGetUniformLocation(shader, "ModelMatrix");
    unsigned int viewMatrixLoc = glGetUniformLocation(shader, "ViewMatrix");
    unsigned int inverseTransposedModelMatrixLoc = glGetUniformLocation(shader, "InverseTransposedModelMatrix");
    unsigned int lightPositionLoc = glGetUniformLocation(shader, "LightPosition");

    unsigned int texture0Loc = glGetUniformLocation(shader, "texture0");

    glUniformMatrix4fv(mvpLoc, 1, GL_FALSE, &mvp[0][0]);

    // Ship the light parameter data to GPU.
    glm::mat4 inverseModelMatrix = glm::inverse(viewMatrix * modelMatrix);
    glUniformMatrix4fv(modelMatrixLoc, 1, GL_FALSE, &modelMatrix[0][0]);
    glUniformMatrix4fv(viewMatrixLoc, 1, GL_FALSE, &viewMatrix[0][0]);
    glUniformMatrix4fv(inverseTransposedModelMatrixLoc, 1, GL_TRUE, &inverseModelMatrix[0][0]);
    glUniform3fv(lightPositionLoc, 1, &lightPosition[0]);

    glUniform1i(texture0Loc, 0);

    material.apply(shader);

    sf::Texture::bind(&texture);

    model.render();

    sf::Texture::bind(nullptr);
  }
}

MapRenderer::MapRenderer(Map* tilemap)
  : m_tilemap(tilemap)
{
  m_lightShader.loadFromMemory(Shader_LightShader_Vertex, Shader_LightShader_Fragment);

  auto tileImages = m_tilemap->getTilesetImages();
  for (const auto& img : tileImages)
  {
    sf::Texture texture;
    texture.loadFromImage(img);

    m_textures.push_back(texture);
  }
}

void MapRenderer::render(sf::RenderTarget& target, const glm::mat4& projectionMatrix, const gamelib::Camera& camera) const
{
  for (int y = 0; y < m_tilemap->getHeight(); y++)
  {
    for (int x = 0; x < m_tilemap->getWidth(); x++)
    {
      if (Tile* tile = m_tilemap->getTileAt(x, y, "wall"))
      {
        if (tile->tileId != -1)
        {
          glm::mat4 modelMatrix{1.f};
          glm::mat4 mvp{1.f};

          modelMatrix *= glm::translate(glm::vec3(static_cast<float>(x), 0, static_cast<float>(y)));
          mvp = projectionMatrix * camera.viewMatrix() * modelMatrix;

          _draw_mesh(mvp, modelMatrix, camera.viewMatrix(), camera.position, gamelib::Material::getDefaultMaterial(), m_wallCube, m_lightShader, m_textures[tile->tileId]);
        }
      }

      if (Tile* tile = m_tilemap->getTileAt(x, y, "floor"))
      {
        if (tile->tileId != -1)
        {
          glm::mat4 modelMatrix{1.f};
          glm::mat4 mvp{1.f};

          modelMatrix *= glm::translate(glm::vec3(static_cast<float>(x), 1.f, static_cast<float>(y)));
          mvp = projectionMatrix * camera.viewMatrix() * modelMatrix;

          _draw_mesh(mvp, modelMatrix, camera.viewMatrix(), camera.position, gamelib::Material::getDefaultMaterial(), m_wallCube, m_lightShader, m_textures[tile->tileId]);
        }
      }

      if (Tile* tile = m_tilemap->getTileAt(x, y, "ceiling"))
      {
        if (tile->tileId != -1)
        {
          glm::mat4 modelMatrix{1.f};
          glm::mat4 mvp{1.f};

          modelMatrix *= glm::translate(glm::vec3(static_cast<float>(x), -1.f, static_cast<float>(y)));
          mvp = projectionMatrix * camera.viewMatrix() * modelMatrix;

          _draw_mesh(mvp, modelMatrix, camera.viewMatrix(), camera.position, gamelib::Material::getDefaultMaterial(), m_wallCube, m_lightShader, m_textures[tile->tileId]);
        }
      }
    }
  }
}

void MapRenderer::initMeshes()
{
  const float size = 0.5f;

  const glm::vec3 back_v1{-size, -size, -size};
  const glm::vec3 back_v2{-size, size, -size};
  const glm::vec3 back_v3{size, size, -size};
  const glm::vec3 back_v4{size, -size, -size};

  const glm::vec3 front_v1{-size, -size, size};
  const glm::vec3 front_v2{-size, size, size};
  const glm::vec3 front_v3{size, size, size};
  const glm::vec3 front_v4{size, -size, size};

  const glm::vec3 left_v1{-size, -size, -size};
  const glm::vec3 left_v2{-size, size, -size};
  const glm::vec3 left_v3{-size, size, size};
  const glm::vec3 left_v4{-size, -size, size};

  const glm::vec3 right_v1{size, -size, -size};
  const glm::vec3 right_v2{size, size, -size};
  const glm::vec3 right_v3{size, size, size};
  const glm::vec3 right_v4{size, -size, size};

  const glm::vec3 top_v1{-size, -size, -size};
  const glm::vec3 top_v2{-size, -size, size};
  const glm::vec3 top_v3{size, -size, size};
  const glm::vec3 top_v4{size, -size, -size};

  const glm::vec3 bottom_v1{-size, size, -size};
  const glm::vec3 bottom_v2{-size, size, size};
  const glm::vec3 bottom_v3{size, size, size};
  const glm::vec3 bottom_v4{size, size, -size};

  const glm::vec3 back_normal = _get_normal(back_v1, back_v2, back_v3);
  const glm::vec3 front_normal = _get_normal(front_v1, front_v2, front_v3);
  const glm::vec3 left_normal = _get_normal(left_v1, left_v2, left_v3);
  const glm::vec3 right_normal = _get_normal(right_v1, right_v2, right_v3);
  const glm::vec3 top_normal = _get_normal(top_v1, top_v2, top_v3);
  const glm::vec3 bottom_normal = _get_normal(bottom_v1, bottom_v2, bottom_v3);

  std::vector<float> vertices =
  {
    // Back
    back_v1.x, back_v1.y, back_v1.z,
    back_v2.x, back_v2.y, back_v2.z,
    back_v3.x, back_v3.y, back_v3.z,
    back_v4.x, back_v4.y, back_v4.z,

    // Front
    front_v1.x, front_v1.y, front_v1.z,
    front_v2.x, front_v2.y, front_v2.z,
    front_v3.x, front_v3.y, front_v3.z,
    front_v4.x, front_v4.y, front_v4.z,

    // Left
    left_v1.x, left_v1.y, left_v1.z,
    left_v2.x, left_v2.y, left_v2.z,
    left_v3.x, left_v3.y, left_v3.z,
    left_v4.x, left_v4.y, left_v4.z,

    // Right
    right_v1.x, right_v1.y, right_v1.z,
    right_v2.x, right_v2.y, right_v2.z,
    right_v3.x, right_v3.y, right_v3.z,
    right_v4.x, right_v4.y, right_v4.z,

    // Top
    top_v1.x, top_v1.y, top_v1.z,
    top_v2.x, top_v2.y, top_v2.z,
    top_v3.x, top_v3.y, top_v3.z,
    top_v4.x, top_v4.y, top_v4.z,

    // Bottom
    bottom_v1.x, bottom_v1.y, bottom_v1.z,
    bottom_v2.x, bottom_v2.y, bottom_v2.z,
    bottom_v3.x, bottom_v3.y, bottom_v3.z,
    bottom_v4.x, bottom_v4.y, bottom_v4.z
  };

  std::vector<float> normals =
  {
    back_normal.x, back_normal.y, back_normal.z,
    back_normal.x, back_normal.y, back_normal.z,
    back_normal.x, back_normal.y, back_normal.z,
    back_normal.x, back_normal.y, back_normal.z,

    front_normal.x, front_normal.y, front_normal.z,
    front_normal.x, front_normal.y, front_normal.z,
    front_normal.x, front_normal.y, front_normal.z,
    front_normal.x, front_normal.y, front_normal.z,

    left_normal.x, left_normal.y, left_normal.z,
    left_normal.x, left_normal.y, left_normal.z,
    left_normal.x, left_normal.y, left_normal.z,
    left_normal.x, left_normal.y, left_normal.z,

    right_normal.x, right_normal.y, right_normal.z,
    right_normal.x, right_normal.y, right_normal.z,
    right_normal.x, right_normal.y, right_normal.z,
    right_normal.x, right_normal.y, right_normal.z,

    top_normal.x, top_normal.y, top_normal.z,
    top_normal.x, top_normal.y, top_normal.z,
    top_normal.x, top_normal.y, top_normal.z,
    top_normal.x, top_normal.y, top_normal.z,

    bottom_normal.x, bottom_normal.y, bottom_normal.z,
    bottom_normal.x, bottom_normal.y, bottom_normal.z,
    bottom_normal.x, bottom_normal.y, bottom_normal.z,
    bottom_normal.x, bottom_normal.y, bottom_normal.z,
  };

  std::vector<float> uvs =
  {
    // Back
    0, 0,
    0, 1,
    1, 1,
    1, 0,

    // Front
    1, 1,
    1, 0,
    0, 0,
    0, 1,

    // Left
    0, 0,
    0, 1,
    1, 1,
    1, 0,

    // Right
    1, 1,
    1, 0,
    0, 0,
    0, 1,

    // Top
    0, 0,
    0, 1,
    1, 1,
    1, 0,

    // Bottom
    1, 1,
    1, 0,
    0, 0,
    0, 1
  };

  gamelib::vertex_pack vertex_data;
  vertex_data.vertices = vertices;
  vertex_data.normals = normals;
  vertex_data.uvs = uvs;

  m_wallCube.init(vertex_data, GL_QUADS);
}

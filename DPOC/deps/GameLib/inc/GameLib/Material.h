#ifndef MATERIAL_H
#define MATERIAL_H

#include <glm/glm.hpp>

#include <GameLib/Shader.h>

namespace gamelib
{

struct Material
{
  glm::vec3 Ka;
  glm::vec3 Kd;
  glm::vec3 Ks;

  Material();
  Material(const glm::vec3& _Ka, const glm::vec3& _Kd, const glm::vec3& _Ks);
  void apply(const gamelib::Shader& shader) const;

  static const Material& getDefaultMaterial();
};

}

#endif

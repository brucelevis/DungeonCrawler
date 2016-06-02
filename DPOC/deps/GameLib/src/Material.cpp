#include <GL/glew.h>
#include <GameLib/Material.h>

namespace gamelib
{

Material::Material()
{}

Material::Material(const glm::vec3& _Ka, const glm::vec3& _Kd, const glm::vec3& _Ks)
  : Ka(_Ka), Kd(_Kd), Ks(_Ks)
{
}

void Material::apply(const gamelib::Shader& shader) const
{
  unsigned int Ka_Loc = glGetUniformLocation(shader, "Ka");
  unsigned int Kd_Loc = glGetUniformLocation(shader, "Kd");
  unsigned int Ks_Loc = glGetUniformLocation(shader, "Ks");

  glUniform3fv(Ka_Loc, 1, &Ka[0]);
  glUniform3fv(Kd_Loc, 1, &Kd[0]);
  glUniform3fv(Ks_Loc, 1, &Ks[0]);
}

const Material& Material::getDefaultMaterial()
{
  static glm::vec3 Ka { 0, 0, 0 };
  static glm::vec3 Kd { 0, 0, 0 };
  static glm::vec3 Ks { 0, 0, 0 };

  static Material material { Ka, Kd, Ks };
  return material;
}

}

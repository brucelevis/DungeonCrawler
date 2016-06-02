#include <cmath>
#include <GameLib/Camera.h>

namespace gamelib
{
  Camera::Camera(const glm::vec3& _position, float _angle)
    : position(_position), 
      horizontal_angle(_angle),
      vertical_angle(0.f)
  {
  }

  glm::mat4 Camera::viewMatrix() const
  {
    return glm::lookAt(position, position + direction(), up());
  }

  glm::vec3 Camera::direction() const
  {
    return glm::vec3 
    {
      cos(vertical_angle) * sin(horizontal_angle),
      sin(vertical_angle),
      cos(vertical_angle) * cos(horizontal_angle)
    };
  }

  glm::vec3 Camera::right() const
  {
    return glm::vec3
    {
      sin(horizontal_angle - glm::pi<float>() / 2.f),
      0,
      cos(horizontal_angle - glm::pi<float>() / 2.f)
    };
  }

  glm::vec3 Camera::up() const
  {
    return glm::cross( right(), direction() );
  }

  void Camera::moveForward(float speed, float timestep)
  {
    position += direction() * timestep * speed;
  }

  void Camera::moveBackward(float speed, float timestep)
  {
    moveForward(-speed, timestep);
  }

  void Camera::strafeRight(float speed, float timestep)
  {
    position += right() * timestep * speed;
  }

  void Camera::strafeLeft(float speed, float timestep)
  {
    strafeRight(-speed, timestep);
  }
}

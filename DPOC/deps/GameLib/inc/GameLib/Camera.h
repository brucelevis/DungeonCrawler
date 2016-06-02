#ifndef GAMELIB_CAMERA_H
#define GAMELIB_CAMERA_H

#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace gamelib
{
  struct Camera
  {
    glm::vec3 position;

    float horizontal_angle;
    float vertical_angle;

    Camera(const glm::vec3& _position, float _angle);

    glm::mat4 viewMatrix() const;

    glm::vec3 direction() const;

    glm::vec3 right() const;

    glm::vec3 up() const;

    void moveForward(float speed, float timestep);

    void moveBackward(float speed, float timestep);

    void strafeRight(float speed, float timestep);

    void strafeLeft(float speed, float timestep);
  };
}

#endif

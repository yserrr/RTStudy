//
// Created by ljh on 25. 9. 7..
//

#ifndef MYPROJECT_RAY_H
#define MYPROJECT_RAY_H
#include "glm/vec3.hpp"
//Ray = origin+ t* direction

struct Ray{
public:
  Ray(const glm::vec3 &origin, const glm::vec3 &direction) : orig(origin), dir(direction) {}
  const glm::vec3 &origin() const
  {
    return orig;
  }
  const glm::vec3 &direction() const
  {
    return dir;
  }
  glm::vec3 at(float t) const
  {
    return orig + t * dir;
  }
  glm::vec3 orig;
  glm::vec3 dir;
};

#endif //MYPROJECT_RAY_H
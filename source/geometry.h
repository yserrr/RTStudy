//
// Created by ljh on 25. 9. 9..
//

#ifndef MYPROJECT_GEOMETRY_H
#define MYPROJECT_GEOMETRY_H
#include <glm/glm.hpp>
#include <memory>
#include <vector>

struct Ray
{
  Ray(const glm::vec3& origin, const glm::vec3& direction) : orig(origin), dir(direction)
  {}

  const glm::vec3& origin() const
  {
    return orig;
  }

  const glm::vec3& direction() const
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

enum class PrimitiveType
{
  Sphere,
  Triangle
};

struct HitRecord
{
  float t = 0;
  bool hit = false;
  glm::vec3 p = glm::vec3(0, 0, 0);
  glm::vec3 normal = glm::vec3(0, 0, 0);
  bool frontFace = false;
  PrimitiveType type = PrimitiveType::Sphere;

  inline void set_face_normal(const Ray& r, const glm::vec3& outwardNormal)
  {
    frontFace = glm::dot(r.dir, outwardNormal) < 0;
    normal = frontFace ? outwardNormal : -outwardNormal;
  }
};


struct Interval
{
  float tMin = 0.01;
  float tMax = std::numeric_limits<float>::max();
};

struct Hittable
{
  virtual ~Hittable() = default;
  virtual bool isHit(const Ray& r, Interval& interval, HitRecord& rec) const = 0;
};


struct Sphere : public Hittable
{
  glm::vec3 center;
  float radius;

  Sphere(const glm::vec3& c, float r) : center(c), radius(r)
  {}

  virtual bool isHit(const Ray& r, Interval& interval, HitRecord& rec) const override
  {
    glm::vec3 oc = r.orig - center;
    float a = glm::dot(r.dir, r.dir);
    float b = glm::dot(oc, r.dir);
    float c = glm::dot(oc, oc) - radius * radius;
    float discriminant = b * b - a * c;

    if (discriminant > 0)
    {
      float root = sqrt(discriminant);
      float temp = (-b - root) / a;
      if (temp < interval.tMax && temp > interval.tMin)
      {
        rec.t = temp;
        rec.p = r.at(rec.t);
        glm::vec3 outward_normal = (rec.p - center) / radius;
        rec.set_face_normal(r, outward_normal);
        interval.tMax = temp;
        rec.hit = true;
        return true;
      }
      temp = (-b + root) / a;
      if (temp < interval.tMax && temp > interval.tMin)
      {
        rec.t = temp;
        rec.p = r.at(rec.t);
        glm::vec3 outward_normal = (rec.p - center) / radius;
        rec.set_face_normal(r, outward_normal);
        interval.tMax = temp;
        rec.hit = true;
        return true;
      }
    }
    return false;
  }
};

//struct HittableList : public Hittable{
//  std::vector<std::shared_ptr<Hittable> > objects;
//
//  void add(std::shared_ptr<Hittable> object)
//  {
//    objects.push_back(object);
//  }
//
//  virtual bool isHit(const Ray &r, float t_min, float t_max, HitRecord &rec) const override
//  {
//    HitRecord temp_rec;
//    bool hit_anything = false;
//    float closest_so_far = t_max;
//
//    for (const auto &object: objects)
//    {
//      if (object->isHit(r, t_min, closest_so_far, temp_rec))
//      {
//        hit_anything = true;
//        closest_so_far = temp_rec.t;
//        rec = temp_rec;
//      }
//    }
//    return hit_anything;
//  }
//};
//
//struct Triangle : public Hittable{
//  glm::vec3 v0, v1, v2; // 꼭짓점
//  glm::vec3 normal;
//
//  Triangle(const glm::vec3 &a, const glm::vec3 &b, const glm::vec3 &c)
//  : v0(a), v1(b), v2(c)
//  {
//    normal = glm::normalize(glm::cross(v1 - v0, v2 - v0));
//  }
//
//  virtual bool isHit(const Ray &r, float t_min, float t_max, HitRecord &rec) const override
//  {
//    const float EPSILON = 1e-8f;
//    glm::vec3 edge1 = v1 - v0;
//    glm::vec3 edge2 = v2 - v0;
//
//    glm::vec3 h = glm::cross(r.dir, edge2);
//    float a = glm::dot(edge1, h);
//
//    if (fabs(a) < EPSILON) return false; // 레이가 평행
//
//    float f = 1.0f / a;
//    glm::vec3 s = r.orig - v0;
//    float u = f * glm::dot(s, h);
//    if (u < 0.0f || u > 1.0f) return false;
//
//    glm::vec3 q = glm::cross(s, edge1);
//    float v = f * glm::dot(r.dir, q);
//    if (v < 0.0f || u + v > 1.0f) return false;
//
//    float t = f * glm::dot(edge2, q);
//    if (t < t_min || t > t_max) return false;
//    rec.t = t;
//    rec.p = r.at(t);
//    rec.set_face_normal(r, normal);
//
//    return true;
//  }
//};
//
#endif //MYPROJECT_GEOMETRY_H

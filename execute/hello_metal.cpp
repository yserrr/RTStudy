//
// Created by ljh on 25. 9. 9..
#include <iostream>
#include <memory>
#include <fstream>
#include <glm/glm.hpp>
#include <ctime>
#include <filesystem>
#include <algorithm>
#include <geometry.h>
#include <stdint.h>
#include <random>
#include <spdlog/spdlog.h>

glm::vec3 randomUnitVector()
{
  std::random_device rd;
  std::mt19937 gen(rd());
  std::uniform_real_distribution<float> dist(-1.0f , 1.0f);
  float x = dist(gen);
  float y = dist(gen);
  float z = dist(gen);
  glm::vec3 randVec3(x, y, z);
  randVec3 = glm::normalize(randVec3);
  return randVec3;
}

glm::vec3 background(const Ray& ray)
{
  glm::vec3 sky = (1 - ray.dir.y) * glm::vec3(1, 1, 1) + (ray.dir.y) * glm::vec3(0.5, 0.7, 1.0);
  return sky;
}

bool hitSphere(Ray ray, glm::vec3 center, float radius)
{
  glm::vec3 oc = center - ray.origin();
  float a = dot(ray.direction(), ray.direction());
  float b = -2.0 * dot(oc, ray.direction());
  float c = dot(oc, oc) - radius * radius;
  float discriminant = b * b - 4 * a * c;
  if (discriminant < 0) return false;
  return true;
}

glm::vec3 trace(Ray ray, const std::vector<Hittable*>& world, uint32_t depth)
{
  glm::vec3 color(0, 0, 0);
  if (depth == 0) return color;
  Interval interval = {};
  HitRecord record;
  for (auto sphere : world)
  {
    sphere->isHit(ray, interval, record);
  }
  if (record.hit)
  {
    glm::vec3 normal = record.normal;
    normal = glm::normalize(normal);
    glm::vec3 reflected  =ray.direction() - 2.0f * float(glm::dot(ray.direction(), normal)) * normal ;
    reflected = glm::normalize(reflected);
    glm::vec3 orig = record.p;
    Ray scatter(orig, reflected);
    if(glm::dot(scatter.direction(), normal) >0)
      return 0.8f* trace(scatter, world, depth-1);
  }
  return background(ray);
}


int main()
{
  std::string outputDir = "frameBuffer";
  std::filesystem::create_directories(outputDir);
  time_t now = time(nullptr);
  char time_buffer[100];
  std::strftime(time_buffer, sizeof(time_buffer), "%Y%m%d_%H%M%S", std::localtime(&now));
  std::string filename = outputDir + std::string("/output_") + time_buffer + ".ppm";
  std::fstream out;
  out.open(filename, std::ios::out);
  if (!out.is_open())
  {
    throw std::runtime_error("Could not open file " + filename);
  }

  int width = 256;
  int height = 256;

  float viewportWidth = 1;
  float viewportHeight = 1;
  float focalLength = 1.0;

  glm::vec3 center = glm::vec3(0, 0, 0);
  glm::vec3 depth = glm::vec3(0.0f, 0.0f, -focalLength);
  glm::vec3 x = glm::vec3(viewportWidth, 0, 0);
  glm::vec3 y = glm::vec3(0, viewportHeight, 0);
  glm::vec3 start = center + depth + (y / 2.0f) - (x / 2.0f);

  Sphere sphere2(glm::vec3(0.0, -100.25, -1.0), 100);
  Sphere sphere1(glm::vec3(0.0, 0.0, -1.0), 0.25);

  std::vector<Hittable*> world;
  world.push_back(&sphere1);
  world.push_back(&sphere2);

  (out) << "P3\n" << width << ' ' << height << "\n255\n";
  for (int i = 0; i < height; i++)
  {
    spdlog::info("left pixels: {} // {}", i, height);
    for (int j = 0; j < width; j++)
    {
      float u = float(j) / (width - 1);
      float v = float(i) / (height - 1);
      glm::vec3 direction = start + (u * x) - (v * y) - center;

      direction = normalize(direction);

      HitRecord record;
      glm::vec3 color = glm::vec3(0, 0, 0);
      uint32_t samplings = 20;
      for (uint32_t k = 0; k< samplings; k++)
      {
        Ray ray(center, direction);
        color += trace(ray, world, k);
      }
      color /= float(samplings);
      out << int(255 * std::clamp(color.x, 0.0f, 1.0f)) << ' '
        << int(255 * std::clamp(color.y, 0.0f, 1.0f)) << ' '
        << int(255 * std::clamp(color.z, 0.0f, 1.0f)) << std::endl;
    }
  }
}

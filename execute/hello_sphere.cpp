#include <iostream>
#include <memory>
#include <fstream>
#include <glm/glm.hpp>
#include <ctime>
#include <filesystem>
#include <algorithm>
#include <geometry.h>


glm::vec3 background(const Ray &ray)
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

  glm::vec3 sphereCenter = glm::vec3(0, 0, -1);
  float radius = 0.25;

  (out) << "P3\n" << width << ' ' << height << "\n255\n";
  for (int i = 0; i < height; i++)
  {
    for (int j = 0; j < width; j++)
    {
      float u = float(j) / (width - 1);
      float v = float(i) / (height - 1);
      glm::vec3 direction = start + (u * x) - (v * y) - center;

      direction = normalize(direction);
      Ray ray(center, direction);

      glm::vec3 color = background(ray);
      if (hitSphere(ray, sphereCenter, radius))
      {
        color = glm::vec3(1, 0, 0);
      }

      out << int(255 * std::clamp(color.x, 0.0f, 1.0f)) << ' '
      << int(255 * std::clamp(color.y, 0.0f, 1.0f)) << ' '
      << int(255 * std::clamp(color.z, 0.0f, 1.0f)) << std::endl;
    }
  }
}
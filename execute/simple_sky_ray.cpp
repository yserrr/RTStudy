#include <iostream>
#include <memory>
#include <fstream>
#include <glm/glm.hpp>
#include <ctime>
#include <filesystem>
#include "../source/Ray.hpp"

glm::vec3 background(const Ray &ray)
{
  glm::vec3 sky = (1 - ray.dir.y) * glm::vec3(1, 1, 1) +
                  (ray.dir.y) * glm::vec3(0.5, 0.7, 1.0);
  return sky;
}

int main()
{
  std::string outputDir = "outputs";
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

  int height = 256;
  int width = 256;

  //view port MODEL
  float viewportWidth = 2;
  float viewportHeight = 2;
  float focalLength = 100.0;
  glm::vec3 center = glm::vec3(0, 0, 0);
  glm::vec3 x = glm::vec3(viewportWidth, 0, 0);
  glm::vec3 y = glm::vec3(0, -viewportHeight, 0);
  glm::vec3 depth = glm::vec3(0.0f, 0.0f, -focalLength);
  //start = lefh high pixel
  glm::vec3 start = center + depth - x / 2.0f - y / 2.0f;
  (out) << "P3\n" << width << ' ' << height << "\n255\n";
  for (int i = 0; i < height; i++)
  {
    for (int j = 0; j < width; j++)
    {
      glm::vec3 color(0, 0, 0);
      float u = float(j) / (width - 1);
      float v = float(i) / (height - 1);
      glm::vec3 direction = start + u * x + v * y;
      direction = direction - center;
      direction = glm::normalize(direction);
      Ray ray(center, direction);
      color = background(ray);

      out
      << int(255 * color.x) << ' '
      << int(255 * color.y) << ' '
      << int(255 * color.z) << std::endl;
    }
  }
}
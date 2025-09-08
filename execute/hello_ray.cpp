#include <iostream>
#include <memory>
#include <fstream>
#include <glm/glm.hpp>
#include <ctime>
#include <filesystem>
int main()
{
  std::string outputDir = "outputs";
  std::filesystem::create_directories(outputDir);
  time_t now = time(nullptr);
  char time_buffer[100];
  std::strftime(time_buffer, sizeof(time_buffer), "%Y%m%d_%H%M%S", std::localtime(&now));
  std::string filename = outputDir+ std::string("/output_") + time_buffer + ".ppm";
  std::fstream out;
  out.open(filename, std::ios::out);
  if (!out.is_open())
  {
    throw std::runtime_error("Could not open file " + filename);
  }
  int width = 400;
  int height = 400;
  (out) << "P3\n" << width << ' ' << height << "\n255\n"; //PPM format header
  for (int i = 0; i < height; i++)
  {
    for (int j = 0; j < width; j++)
    {
      auto r = double(i) / (width - 1);
      auto g = double(j) / (height - 1);
      auto b = 0.0;
      int ir = int(255.999 * r);
      int ig = int(255.999 * g);
      int ib = int(255.999 * b);
      out << ir << ' ' << ig << ' ' << ib << '\n';
    }
  }
}
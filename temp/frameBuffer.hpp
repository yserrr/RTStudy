#ifndef FRAMEBUFFER_HPP
#define FRAMEBUFFER_HPP
#include <memory>
#include <GLFW/glfw3.h>
#include <cstdint>
#include <vector>

class FrameBuffer{
public:
  FrameBuffer() = default;
  ~FrameBuffer() = default;
  void init(int width, int height);
  void update() const;
  bool isEnd();
  GLuint getFrameHandler() const;

private:
  GLuint textureHandler_;
  uint32_t height_;
  uint32_t width_;
  bool dirty_;
  std::vector<uint8_t> pixels;
};
#endif //
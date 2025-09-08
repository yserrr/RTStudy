//
// Created by ljh on 25. 9. 7..
//

#include "../source/frameBuffer.hpp"

void FrameBuffer::init(int width, int height)
{
  width_ = width;
  height_ = height;
  glGenTextures(1, &textureHandler_);
  glBindTexture(GL_TEXTURE_2D, textureHandler_);
  pixels.resize(width_ * height_ * 3);
  glTexImage2D(GL_TEXTURE_2D,
               0,
               GL_RGB,
               width_,
               height_,
               0,
               GL_RGB,
               GL_UNSIGNED_BYTE,
               pixels.data());
}

void FrameBuffer::update() const
{
  glBindTexture(GL_TEXTURE_2D, textureHandler_);
  glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width_, height_, GL_RGB, GL_UNSIGNED_BYTE, pixels.data());
}

GLuint FrameBuffer::getFrameHandler() const
{
  return textureHandler_;
}

bool FrameBuffer::isEnd()
{
  if (dirty_)
  {
    dirty_ = false;
    return true;
  }
  return false;
}
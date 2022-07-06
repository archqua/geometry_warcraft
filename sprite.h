#pragma once
#ifndef SPRITE_H_SENTRY
#define SPRITE_H_SENTRY

#include <stdint.h>

#include "physical_object.h"
#include <vector>
#include <fstream>
#include <cmath>
#include <memory>

using byte = unsigned char;

struct Pixel {
  byte r, g, b, a;
  void toArr(byte arr[4]) const;
  uint32_t toRGBA() const;
  uint32_t toBGRA() const;
};

class Sprite {
  std::vector<Pixel> pixels;
  unsigned height, width;
public:
  Sprite() = default;
  Sprite(unsigned h, unsigned w): pixels(h*w), height(h), width(w) {}
  Sprite(Sprite&& other) = default;
  //   : pixels(std::move(other.pixels)), height(other.height), width(other.width)
  // {}
  Sprite& operator=(Sprite&& other) = default;
  // void operator=(Sprite&& other) {
  //   pixels=std::move(other.pixels); height=other.height; width=other.width;
  // }
  Pixel operator()(unsigned h, unsigned w) const;
  Pixel& operator()(unsigned h, unsigned w);
  unsigned getHeight() const { return height; }
  unsigned getWidth() const { return width; }

  using PixelConstIterator = std::vector<Pixel>::const_iterator;
  using PixelIterator = std::vector<Pixel>::iterator;
  PixelConstIterator pixelsBegin() const { return pixels.begin(); }
  PixelConstIterator pixelsEnd() const { return pixels.end(); }
  PixelIterator pixelsBegin() { return pixels.begin(); }
  PixelIterator pixelsEnd() { return pixels.end(); }

  template <unsigned bitcount = 4>
  static Sprite fromBinaryFile(const char *file);

  class Error {
  };

  class FOError : public Error {
    const char *file;
  public:
    FOError(const char *file): file(file) {}
    const char *what() const { return file; }
  };
  class FRError : public Error {
    const char *file;
  public:
    FRError(const char *file): file(file) {}
    const char *what() const { return file; }
  };
}; // Sprite

template <unsigned bitcount>
Sprite Sprite::fromBinaryFile(const char *file)
{
  std::ifstream fs(file, std::ios::binary);
  if (fs) {
    unsigned height = 0, width = 0;
    byte h[bitcount];
    byte w[bitcount];
    fs.read((char*) h, bitcount);
    fs.read((char*) w, bitcount);
    for (unsigned i = 0; i < bitcount; ++i) {
      height += pow(256, i) * h[bitcount-1-i];
      width += pow(256, i) * w[bitcount-1-i];
    }
    Sprite res(height, width);
    try {
      for (PixelIterator iter = res.pixelsBegin(); iter != res.pixelsEnd(); ++iter) {
        // I don't know why this works anymore
        fs.read((char*) &(iter->r), 1);
        fs.read((char*) &(iter->g), 1);
        fs.read((char*) &(iter->b), 1);
        fs.read((char*) &(iter->a), 1);
      }
    return res;
    } catch (const std::ios_base::failure&) {
      throw FRError(file);
    }
  } else {
    throw FOError(file);
  }
}

class SpriteRef : public std::shared_ptr<Sprite> {
public:
  SpriteRef() = default;
  using Base = std::shared_ptr<Sprite>;
  SpriteRef(std::shared_ptr<Sprite> ptr): Base(std::move(ptr)) {}
  SpriteRef(unsigned h, unsigned w) { *this = std::make_shared<Sprite>(h, w); }
  Pixel operator()(unsigned h, unsigned w) const { return (**this)(h, w); }
  Pixel& operator()(unsigned h, unsigned w) { return (**this)(h, w); }
};

class SpriteObject : public PhysicalObject {
protected:
  Point2d pos;
  float rot;
  SpriteRef sprite;
public:
  SpriteObject() = default;
  // SpriteObject(Point2d pos, float rot, std::shared_ptr<Sprite> sprite)
  //   : pos(pos), rot(rot), sprite(std::move(sprite)) {}
  SpriteObject(Point2d pos, float rot, SpriteRef sprite, std::optional<Box2d> ptl = std::nullopt)
    : PhysicalObject(std::move(ptl)), pos(pos), rot(rot), sprite(std::move(sprite))
  {
    y_frac = pos.y;
    x_frac = pos.x;
  }
  void draw(uint32_t *buffer, unsigned screen_h, unsigned screen_w) override;
  void act(float dt) override;
};

#endif // SPRITE_H_SENTRY

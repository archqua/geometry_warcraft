#pragma once
#ifndef PHYSICAL_OBJECT_SENTRY
#define PHYSICAL_OBJECT_SENTRY

#include "geometry.h"
#include <optional>
#include <stdint.h>

class PhysicalObject {
protected:
  float x_frac = 0, y_frac = 0;
  float x_vel = 0, y_vel = 0;
  float x_acc = 0, y_acc = 0;
  std::optional<Box2d> place_to_live = std::nullopt;
public:
  virtual void draw(uint32_t *buffer, unsigned screen_h, unsigned screen_w) = 0;
  virtual void act(float dt);

  virtual ~PhysicalObject() {};
}; // PhysicalObject

#endif

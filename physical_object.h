#pragma once
#ifndef PHYSICAL_OBJECT_SENTRY
#define PHYSICAL_OBJECT_SENTRY

#include "geometry.h"
#include "semaphore.h"
#include <optional>
#include <future>
#include <list>
#include <stdint.h>

class PhysicalObject {
protected:
  float x_frac = 0, y_frac = 0;
  float x_vel = 0, y_vel = 0;
  float x_acc = 0, y_acc = 0;
  std::optional<Box2d> place_to_live = std::nullopt;
public:
  PhysicalObject() = default;
  PhysicalObject(std::optional<Box2d> ptl): place_to_live(std::move(ptl)) {}
  virtual void draw(uint32_t *buffer, unsigned screen_h, unsigned screen_w) {
    (void)buffer; (void)screen_h; (void)screen_w;
  }
  virtual void act(float dt);
  virtual bool isInsideBox(const Box2d& box);

  virtual ~PhysicalObject() {};
}; // PhysicalObject

using PhysicalObjectHandle = std::unique_ptr<PhysicalObject>;


#endif // PHYSICAL_OBJECT_SENTRY

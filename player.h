#pragma once
#ifndef PLAYER_H_SENTRY
#define PLAYER_H_SENTRY

#include "collider.h"
#include "physical_object.h"
#include "geometry.h"
#include "sprite.h"

class Player : public PhysicalObject, public CollisionObject {
  Point2d pos;
  float rot;
  Sprite sprite;

  Sphere collision_sphere;

public:
  Player(Point2d pos, float rot, Sprite&& sprite, std::optional<Box2d> ptl = std::nullopt)
    : pos(pos), rot(rot), sprite(std::move(sprite))
  {
    place_to_live = ptl;
  }
  void draw(uint32_t *buffer, unsigned screen_h, unsigned screen_w) override;
  void act(float dt) override;
}; // Player


#endif // PLAYER_H_SENTRY

#pragma once
#ifndef PLAYER_H_SENTRY
#define PLAYER_H_SENTRY

#include "physical_object.h"
#include "geometry.h"
#include "sprite.h"

class Player : public PhysicalObject {
  Point2d pos;
  float rot;
  Sprite sprite;
public:
  Player(Point2d pos, float rot, Sprite&& sprite)
    : pos(pos), rot(rot), sprite(std::move(sprite))
  {}
  void draw(uint32_t *buffer, unsigned screen_h, unsigned screen_w) override;
  void act(float dt) override;
}; // Player


#endif // PLAYER_H_SENTRY

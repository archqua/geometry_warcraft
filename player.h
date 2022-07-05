#pragma once
#ifndef PLAYER_H_SENTRY
#define PLAYER_H_SENTRY

#include "collider.h"
#include "physical_object.h"
#include "geometry.h"
#include "sprite.h"
#include "weapon.h"
#include <future>

class Player : public SpriteObject, public CollisionObject {
  Point2d move(float dt);
  void fire(Point2d cursor);
  std::future<void> cooldowner;

  std::unique_ptr<Weapon> weapon;
public:
  Player(Point2d pos, float rot, SpriteRef sprite, std::optional<Box2d> ptl = std::nullopt);
  void act(float dt) override;
  void arm(const std::unique_ptr<Weapon>& weap) { weapon = weap->copy(); }
}; // Player


#endif // PLAYER_H_SENTRY

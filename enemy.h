#pragma once
#ifndef ENEMY_H_SENTRY
#define ENEMY_H_SENTRY

#include "weapon.h"

class Enemy : public ArmedObject {
public:
  Enemy(Point2d pos, float rot, SpriteRef sprite, Cooldowner cdr, std::optional<Box2d> ptl = std::nullopt);
  void addCollisionShape(std::unique_ptr<CollisionShape> cs);
};

namespace enemy{

class Sphere : public Enemy {
protected:
  static constexpr unsigned traction_towards_desired_velocity = 20;
  static constexpr unsigned velocity_noise = 200;
  static constexpr unsigned rotation_noise = 1200;
  static constexpr unsigned desired_velocity = 500;
  static constexpr unsigned rot_vel_downtraction = 5;
  float rot_vel = 0;
public:
  // Sphere(Point2d pos, float rot, SpriteRef sprite, Cooldowner cdr, std::optional<Box2d> ptl = std::nullopt);
  Sphere(Point2d pos, float rot, SpriteRef sprite, Cooldowner cdr, std::optional<Box2d> ptl = std::nullopt, float collision_radius = 50);
  void act(float dt) override;
};

} // enemy



#endif

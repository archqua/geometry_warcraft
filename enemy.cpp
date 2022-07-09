#include "enemy.h"
#include "rand_mx.h"
#include <cmath>
#include <cstdlib>

Enemy::Enemy(Point2d pos, float rot, SpriteRef sprite, Cooldowner cdr, std::optional<Box2d> ptl)
  : ArmedObject(pos, rot, std::move(sprite), cdr, std::move(ptl))
{
  for (auto iter = CollisionObject::origBegin(); iter != CollisionObject::origEnd(); ++iter) {
    iter->setReceiveOn(static_cast<unsigned>(Collider::MaskIdx::enemy));
    iter->setHitOn(static_cast<unsigned>(Collider::MaskIdx::player));
  }
}

void Enemy::addCollisionShape(std::unique_ptr<CollisionShape> cs) {
  CollisionObject::addOrig(Collider(
    std::move(cs),
    hitMask(Collider::Mask::player),
    receiveMask(Collider::Mask::enemy)
  ));
}

namespace enemy {
  
void Sphere::act(float dt) {
  // compute velocity
  float velocity = sqrt(y_vel*y_vel + x_vel*x_vel);
  float noise, fire_ang;
  {
  std::unique_lock rand_lock(rand_mx);
  fire_ang = (rand() % 360) * M_PI / 180.0;
  noise = rand() % velocity_noise;
  noise -= velocity_noise/2;
  velocity += dt * noise;
  velocity += dt * traction_towards_desired_velocity * (desired_velocity - velocity);
  // modify rotation
  noise = rand() % rotation_noise;
  } // release rand_mx
  noise -= rotation_noise/2;
  rot_vel += dt * noise;
  rot_vel -= dt * rot_vel * rot_vel_downtraction;
  rot += dt * rot_vel;
  // set velocity
  y_vel = -cos(rot) * velocity;
  x_vel = -sin(rot) * velocity;
  // move
  Enemy::act(dt);
  // fire
  if (weapon->isReady()) {
    Point2d target = pos + Point2d{
        .y = (int)(sin(fire_ang) * 100),
        .x = (int)(cos(fire_ang) * 100),
    };
    // (void)target;
    fire(target);
  }
}

// Sphere::Sphere(Point2d pos, float rot, SpriteRef sprite, Cooldowner cdr, std::optional<Box2d> ptl)
//   : Enemy(pos, rot, std::move(sprite), cdr, std::move(ptl))
// { }
Sphere::Sphere(Point2d pos, float rot, SpriteRef sprite, Cooldowner cdr, std::optional<Box2d> ptl, float collision_radius)
  : Enemy(pos, rot, std::move(sprite), cdr, std::move(ptl))
{
  Enemy::addCollisionShape(std::make_unique<::Sphere>(Point2d{.y=0, .x=0}, collision_radius));
}

}

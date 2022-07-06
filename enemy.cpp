#include "enemy.h"
#include <cmath>
#include <cstdlib>

Enemy::Enemy(Point2d pos, float rot, SpriteRef sprite, std::optional<Box2d> ptl)
  : ArmedObject(pos, rot, std::move(sprite), std::move(ptl))
{
  for (std::unique_ptr<Collider>& col : static_cast<CollisionObject&>(*this)) {
    col->setReceiveOn(static_cast<unsigned>(Collider::Mask::player));
  }
}

namespace enemy {
  
void Sphere::act(float dt) {
  // compute velocity
  float velocity = sqrt(y_vel*y_vel + x_vel*x_vel);
  float noise = rand() % velocity_noise;
  noise -= velocity_noise/2;
  velocity += dt * noise;
  velocity += dt * traction_towards_desired_velocity * (desired_velocity - velocity);
  // modify rotation
  noise = rand() % rotation_noise;
  noise -= rotation_noise/2;
  rot_vel += dt * noise;
  rot_vel -= dt * rot_vel * rot_vel_downtraction;
  rot += dt * rot_vel;
  // set velocity
  y_vel = sin(rot) * velocity;
  x_vel = cos(rot) * velocity;
  // move
  PhysicalObject::act(dt);
  pos.y = y_frac;
  pos.x = x_frac;
  // fire
  if (weapon->isReady()) {
    float fire_ang = (rand() % 360) * M_PI / 180.0;
    Point2d target = pos + Point2d{
        .y = (int)(sin(fire_ang) * 100),
        .x = (int)(cos(fire_ang) * 100),
    };
    fire(target);
  }
}

Sphere::Sphere(Point2d pos, float rot, SpriteRef sprite, std::optional<Box2d> ptl)
  : Enemy(pos, rot, std::move(sprite), std::move(ptl))
{ }

}

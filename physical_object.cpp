#include "physical_object.h"


void PhysicalObject::act(float dt) {
  float _x_vel = x_vel;
  float _y_vel = y_vel;
  x_vel += x_acc * dt;
  y_vel += y_acc * dt;
  x_frac += 0.5 * (_x_vel + x_vel) * dt;
  y_frac += 0.5 * (_y_vel + y_vel) * dt;
  if (place_to_live) {
    const Box2d& box = *place_to_live;
    x_frac = x_frac < box.lt.x ? box.lt.x : x_frac;
    y_frac = y_frac < box.lt.y ? box.lt.y : y_frac;
    x_frac = x_frac > box.rb.x ? box.rb.x : x_frac;
    y_frac = y_frac > box.rb.y ? box.rb.y : y_frac;
  }
}

bool PhysicalObject::isInsideBox(const Box2d& box) {
  return y_frac >= box.lt.y && y_frac < box.rb.y && x_frac >= box.lt.x && x_frac < box.rb.x;
}

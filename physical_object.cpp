#include "physical_object.h"


void PhysicalObject::act(float dt) {
  float _x_vel = x_vel;
  float _y_vel = y_vel;
  x_vel += x_acc * dt;
  y_vel += y_acc * dt;
  x_frac += 0.5 * (_x_vel + x_vel) * dt;
  y_frac += 0.5 * (_y_vel + y_vel) * dt;
}

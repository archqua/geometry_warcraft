#include "weapon.h"
#include <cmath>
// #include <chrono>
#include <unistd.h>



void Armory::load(Weapon::ProjectileInserter pi) {
  weapon::projectile::Simple::sprite = std::make_shared<Sprite>(Sprite::fromBinaryFile("sprites/bullet.b"));
  weapons.push_back(std::make_unique<weapon::Simple>(pi));
}

namespace weapon {

void cooldown_fn(Simple& simple, unsigned cd /* ms */) {
  // std::chrono::milliseconds dur(cd);
  simple.setUnready();
  // void std::this_thread::sleep_for(dur);
  // TODO will it never break??
  usleep(cd * 1000);
  simple.setReady();
}
std::future<void> Simple::fire(Point2d from, Point2d to, Point2d base_velocity) {
  Point2d direction = to - from;
  float rot = direction.angle(Point2d{.y = -1, .x = 0});
  auto projectile = std::make_unique<projectile::Simple>(from, rot);
  float normie = 1/direction.length();
  projectile->y_vel = base_velocity.y + (normie * projectile->velocity) * direction.y;
  projectile->x_vel = base_velocity.x + (normie * projectile->velocity) * direction.x;
  projectile->pos = from;
  projectile->y_frac = from.y;
  projectile->x_frac = from.x;
  projectile_inserter = std::move(projectile);
  return std::async(cooldown_fn, std::ref(*this), cooldown);
}

std::unique_ptr<Weapon> Simple::copy() const {
  return std::make_unique<Simple>(*this);
}

namespace projectile {

SpriteRef Simple::sprite;

} // projectile

bool Simple::isReady() {
  auto l = read();
  return ready;
}

Simple::Lock Simple::read() {
  return Lock(mx);
}

Simple::Lock Simple::write() {
  return Lock(mx);
}

Simple& Simple::setUnready() {
  auto l = write();
  ready = false;
  return *this;
}

Simple& Simple::setReady() {
  auto l = write();
  ready = true;
  return *this;
}

} // weapon

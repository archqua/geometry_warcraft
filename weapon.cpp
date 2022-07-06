#include "weapon.h"
#include <cmath>
#include <cstdlib>
// #include <chrono>
#include <unistd.h>



void Armory::load(Weapon::ProjectileInserter pi) {
  weapon::projectile::Simple::sprite = std::make_shared<Sprite>(Sprite::fromBinaryFile("sprites/bullet.b"));
  weapons.push_back(std::make_unique<weapon::Simple>(pi));
  weapons.push_back(std::make_unique<weapon::Random>(pi));
  weapons.push_back(std::make_unique<weapon::Random>(pi, 50, 250));
}

namespace weapon {

void cooldown_fn(Basic& basic, unsigned cd /* ms */) {
  // std::chrono::milliseconds dur(cd);
  basic.setUnready();
  // void std::this_thread::sleep_for(dur);
  // TODO will it never break??
  usleep(cd * 1000);
  basic.setReady();
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
std::future<void> Random::fire(Point2d from, Point2d to, Point2d base_velocity) {
  Point2d direction = to - from;
  float rot = direction.angle(Point2d{.y = -1, .x = 0});
  unsigned velocity = (unsigned)std::rand() % (hi_velocity - lo_velocity);
  velocity += lo_velocity;
  auto projectile = std::make_unique<projectile::Simple>(from, rot, velocity);
  float normie = 1/direction.length();
  projectile->y_vel = base_velocity.y + (normie * projectile->velocity) * direction.y;
  projectile->x_vel = base_velocity.x + (normie * projectile->velocity) * direction.x;
  projectile->pos = from;
  projectile->y_frac = from.y;
  projectile->x_frac = from.x;
  projectile_inserter = std::move(projectile);
  unsigned cooldown = (unsigned)std::rand() % (hi_cooldown - lo_cooldown);
  cooldown += lo_cooldown;
  return std::async(cooldown_fn, std::ref(*this), cooldown);
}

std::unique_ptr<Weapon> Simple::copy() const {
  return std::make_unique<Simple>(*this);
}
std::unique_ptr<Weapon> Random::copy() const {
  return std::make_unique<Random>(*this);
}

namespace projectile {

SpriteRef Simple::sprite;

} // projectile

bool Basic::isReady() {
  auto l = read();
  return ready;
}

Basic::Lock Basic::read() {
  return Lock(mx);
}

Basic::Lock Basic::write() {
  return Lock(mx);
}

Basic& Basic::setUnready() {
  auto l = write();
  ready = false;
  return *this;
}

Basic& Basic::setReady() {
  auto l = write();
  ready = true;
  return *this;
}

} // weapon



void ArmedObject::fire(Point2d cursor) {
  if (weapon && weapon->isReady()) {
    cooldowner = weapon->fire(pos, cursor, Point2d{.y = (int)y_vel, .x = (int)x_vel});
  }
}


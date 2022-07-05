#pragma once
#ifndef WEAPON_H_SENTRY
#define WEAPON_H_SENTRY


#include "geometry.h"
#include "physical_object.h"
#include "sprite.h"
#include <list>
#include <iterator>
#include <vector>
#include <future>
#include <mutex>




class Weapon {
public:
  using ProjectileInserter = std::insert_iterator<std::list<std::unique_ptr<PhysicalObject>>>;
protected:
  ProjectileInserter projectile_inserter;
public:
  Weapon(ProjectileInserter pi): projectile_inserter(pi) {}
  virtual std::future<void> fire(Point2d from, Point2d to, Point2d base_velocity) {
    (void)from; (void)to; (void)base_velocity;
    return std::async([](){});
  }
  virtual bool isReady() = 0;
  virtual std::unique_ptr<Weapon> copy() const = 0;
  virtual ~Weapon() {}
};

class Armory {
  std::vector<std::unique_ptr<Weapon>> weapons;
public:
  void load(Weapon::ProjectileInserter);
  class Error {};
  class FOError : public Error {
    const char *file;
  public:
    FOError(const char *file): file(file) {}
    const char *what() const { return file; }
  };
  class FRError : public Error {
    const char *file;
  public:
    FRError(const char *file): file(file) {}
    const char *what() const { return file; }
  };
  const std::unique_ptr<Weapon>& operator[](size_t i) const { return weapons[i]; }
};

namespace weapon {

class Simple : public Weapon {
  static constexpr unsigned default_cooldown = 300; // ms
  std::mutex mx;
  bool ready = true;
public:
  const unsigned cooldown = default_cooldown;
  Simple(Weapon::ProjectileInserter pi, unsigned cd = default_cooldown): Weapon(pi), cooldown(cd) {}
  Simple(const Simple& other): Weapon(other.projectile_inserter), cooldown(other.cooldown) {}
  std::future<void> fire(Point2d from, Point2d to, Point2d base_velocity) override;
  bool isReady() override;
  std::unique_ptr<Weapon> copy() const override;
  using Lock = std::lock_guard<std::mutex>;
  Lock write();
  Lock read();
  Simple& setUnready();
  Simple& setReady();
};

namespace projectile {

class Simple : public SpriteObject {
  static SpriteRef sprite;
  static constexpr unsigned default_velocity = 2000;
public:
  const unsigned velocity = default_velocity;
  Simple(Point2d pos, float rot, unsigned vel = default_velocity)
    : SpriteObject(pos, rot, sprite), velocity(vel) {}
  friend weapon::Simple;
  friend Armory;
};

} // projectile

} // weapon





#endif

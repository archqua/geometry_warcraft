#pragma once
#ifndef WEAPON_H_SENTRY
#define WEAPON_H_SENTRY


#include "geometry.h"
#include "physical_object.h"
#include "sprite.h"
#include "collider.h"
#include "semaphore.h"
#include <list>
#include <iterator>
#include <vector>
#include <future>
#include <mutex>




class Weapon {
public:
  using ProjectileInserter = PhysicalObjectManager::ObjectInsertIterator;
  // using ProjectileInserter = std::insert_iterator<std::list<std::unique_ptr<PhysicalObject>>>;
protected:
  // ProjectileInserter projectile_inserter;
  PhysicalObjectManager& phom;
protected:
  Semaphore *availability_semaphore = nullptr;
  Semaphore *semaphore_removability_semaphore = nullptr;
public:
  // Weapon(ProjectileInserter pi): projectile_inserter(pi) {}
  Weapon(PhysicalObjectManager& phom): phom(phom) {}
  Weapon(const Weapon&) = default;
  virtual std::future<void> fire(Point2d from, Point2d to, Point2d base_velocity) {
    (void)from; (void)to; (void)base_velocity;
    return std::async([](){});
  }
  virtual bool isReady() = 0;
  virtual std::unique_ptr<Weapon> copy() const = 0;
  virtual ~Weapon() {}
  std::pair<Semaphore*, Semaphore*> getUtilSemaphores() const {
    return std::make_pair(availability_semaphore, semaphore_removability_semaphore);
  }
  void setUtilSemaphores(const std::pair<Semaphore*, Semaphore*>& sw) {
    availability_semaphore = sw.first;
    semaphore_removability_semaphore = sw.second;
  }
};

class ArmedObject : public CollisionObject {
protected:
  virtual void fire(Point2d cursor);
  using Cooldowner = PhysicalObjectManager::FutureInsertIterator;
  Cooldowner cooldowner;
  // std::future<void> cooldowner;
  std::unique_ptr<Weapon> weapon;
public:
  // ArmedObject() = default;
  ArmedObject(Point2d pos, float rot, SpriteRef sprite, Cooldowner cdr, std::optional<Box2d> ptl = std::nullopt)
    : CollisionObject(pos, rot, std::move(sprite), std::move(ptl)), cooldowner(cdr) {}
  virtual void arm(const std::unique_ptr<Weapon>& weap) {
    std::optional<std::pair<Semaphore*, Semaphore*>> sw(std::nullopt);
    if (weapon) {
      sw = weapon->getUtilSemaphores();
    }
    weapon = weap->copy();
    if (sw) {
      weapon->setUtilSemaphores(*sw);
    }
  }
  virtual void weaponSetUtilSemaphores(const std::pair<Semaphore*, Semaphore*> sw) {
    weapon->setUtilSemaphores(sw);
  }
};

class Armory {
  // using WeaponHandle = PhysicalObjectManager::ObjectSmartHandle;
  using WeaponHandle = std::unique_ptr<Weapon>;
  std::vector<WeaponHandle> weapons;
  // PhysicalObjectManager *phom = nullptr;
public:
  void load(PhysicalObjectManager *phom);
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
  class SegmentationFault : public Error {
  public:
    const char *what() const { return "Armory: segmentation fault"; }
  };
  const WeaponHandle& operator[](size_t i) const { return weapons[i]; }
};

namespace weapon {

class Basic : public Weapon {
  Semaphore ready_semaphore;
public:
  // Basic(ProjectileInserter pi): Weapon(pi) {}
  Basic(PhysicalObjectManager& phom): Weapon(phom) {}
  // Basic(const Basic& other): Weapon(other.projectile_inserter) {}
  Basic(const Basic& other): Weapon(other.phom) {}
  bool isReady() override;
  // using Lock = std::lock_guard<std::mutex>;
  // Lock write();
  // Lock read();
  Basic& setUnready();
  Basic& setReady();
  class RaceCondition {
  public:
    const char *what() { return "weapon::Simple: race condition"; }
  };
};

class Simple : public Basic {
  static constexpr unsigned default_cooldown = 300; // ms
public:
  const unsigned cooldown = default_cooldown;
  // Simple(Weapon::ProjectileInserter pi, unsigned cd = default_cooldown): Basic(pi), cooldown(cd) {}
  Simple(PhysicalObjectManager& phom, unsigned cd = default_cooldown): Basic(phom), cooldown(cd) {}
  // Simple(const Simple& other): Basic(other.projectile_inserter), cooldown(other.cooldown) {}
  Simple(const Simple& other): Basic(other.phom), cooldown(other.cooldown) {}
  std::future<void> fire(Point2d from, Point2d to, Point2d base_velocity) override;
  std::unique_ptr<Weapon> copy() const override;
};

class Random : public Basic {
  static constexpr unsigned default_lo_cd = 212; // ms
  static constexpr unsigned default_hi_cd = 424; // ms
  static constexpr unsigned default_lo_vel = 1414;
  static constexpr unsigned default_hi_vel = 2828;
public:
  const unsigned lo_cooldown = default_lo_cd;
  const unsigned hi_cooldown = default_hi_cd;
  const unsigned lo_velocity = default_lo_vel;
  const unsigned hi_velocity = default_hi_vel;
  // Random(
  //   Weapon::ProjectileInserter pi,
  //   unsigned lo_cd = default_lo_cd, unsigned hi_cd = default_hi_cd,
  //   unsigned lo_vel = default_lo_vel, unsigned hi_vel = default_hi_vel
  // )
  //   : Basic(pi)
  //   , lo_cooldown(lo_cd), hi_cooldown(hi_cd)
  //   , lo_velocity(lo_vel), hi_velocity(hi_vel)
  // {}
  Random(
    PhysicalObjectManager& phom,
    unsigned lo_cd = default_lo_cd, unsigned hi_cd = default_hi_cd,
    unsigned lo_vel = default_lo_vel, unsigned hi_vel = default_hi_vel
  )
    : Basic(phom)
    , lo_cooldown(lo_cd), hi_cooldown(hi_cd)
    , lo_velocity(lo_vel), hi_velocity(hi_vel)
  {}
  // Random(const Random& other)
  //   : Basic(other.projectile_inserter)
  //   , lo_cooldown(other.lo_cooldown), hi_cooldown(other.hi_cooldown)
  //   , lo_velocity(other.lo_velocity), hi_velocity(other.hi_velocity)
  // {}
  Random(const Random& other)
    : Basic(other.phom)
    , lo_cooldown(other.lo_cooldown), hi_cooldown(other.hi_cooldown)
    , lo_velocity(other.lo_velocity), hi_velocity(other.hi_velocity)
  {}
  std::future<void> fire(Point2d from, Point2d to, Point2d base_velocity) override;
  std::unique_ptr<Weapon> copy() const override;
};

namespace projectile {

class Simple : public CollisionObject {
  static SpriteRef sprite;
  static constexpr unsigned default_velocity = 2000;
public:
  const unsigned velocity = default_velocity;
  Simple(Point2d pos, float rot, unsigned vel = default_velocity)
    : CollisionObject(pos, rot, sprite), velocity(vel) {}
  friend weapon::Simple;
  friend weapon::Random;
  friend Armory;
};

} // projectile

} // weapon




#endif

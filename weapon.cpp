#include "weapon.h"
#include <cmath>
#include <cstdlib>
// #include <chrono>
#include <unistd.h>



void Armory::load(PhysicalObjectManager *phom)
  // : phom(phom)
{
  if (!phom) {
    throw SegmentationFault();
  }
  weapon::projectile::Simple::sprite = std::make_shared<Sprite>(Sprite::fromBinaryFile("sprites/bullet.b"));
  // auto pi = phom->objectPrepender(); // projectile inserter
  // weapons.emplace_back(std::make_unique<weapon::Simple>(pi));
  // weapons.emplace_back(std::make_unique<weapon::Random>(pi));
  // weapons.emplace_back(std::make_unique<weapon::Random>(pi, 50, 250));
  auto bulletrect = Collider(std::make_unique<Rectangle>(
        Box2d{.lt=Point2d{.y=-20, .x=-10,}, .rb=Point2d{.y=20, .x=10,},}
  ));
  weapons.emplace_back(std::make_unique<weapon::Simple>(*phom, bulletrect));
  weapons.emplace_back(std::make_unique<weapon::Random>(*phom, bulletrect));
  weapons.emplace_back(std::make_unique<weapon::Random>(*phom, bulletrect, 50, 250));
}

namespace weapon {

void cooldown_fn(
  Basic& basic, unsigned cd /* ms */,
  Semaphore *availability_semaphore, Semaphore *removability_semaphore
) {
  // std::chrono::milliseconds dur(cd);
  // void std::this_thread::sleep_for(dur);
  // TODO will it never break??
  usleep(cd * 1000);
  if (!availability_semaphore) {
    throw Basic::RaceCondition();
  }
  {
    auto l = availability_semaphore->lock();
    if (availability_semaphore->countUnsafe() > 0) {
      basic.setReady();
    }
    // l.~Lock();
  }
  if (!removability_semaphore) {
    throw Basic::RaceCondition();
  }
  removability_semaphore->dec();
}
// using ProjectileInsertWrap = PhysicalObjectManager::ObjectSmartHandle;
std::future<void> Simple::fire(Point2d from, Point2d to, Point2d base_velocity) {
  // kinda unsafe w\o usage of availability_semaphore
  if (!semaphore_removability_semaphore) {
    throw RaceCondition();
  }
  auto l = semaphore_removability_semaphore->lock();
  if (semaphore_removability_semaphore->countUnsafe() > 0) {
    semaphore_removability_semaphore->incUnsafe();
    Point2d direction = to - from;
    float rot = direction.angle(Point2d{.y = -1, .x = 0});
    auto projectile = std::make_unique<projectile::Simple>(from, rot);
    float normie = 1/direction.length();
    projectile->y_vel = base_velocity.y + (normie * projectile->velocity) * direction.y;
    projectile->x_vel = base_velocity.x + (normie * projectile->velocity) * direction.x;
    projectile->pos = from;
    projectile->y_frac = from.y;
    projectile->x_frac = from.x;
    projectile->addOrig(proj_col);
    // auto wr = ProjectileInsertWrap(std::move(projectile));
    // projectile_inserter = std::move(wr);
    phom.prependObject(std::move(projectile));
    setUnready();
    return std::async(
      cooldown_fn,
      std::ref(*this), cooldown,
      availability_semaphore, semaphore_removability_semaphore
    );
  } else {
    return Basic::fire(from, to, base_velocity); // no-op
  }
}
std::future<void> Random::fire(Point2d from, Point2d to, Point2d base_velocity) {
  // kinda unsafe w\o usage of availability_semaphore
  if (!semaphore_removability_semaphore) {
    throw RaceCondition();
  }
  auto l = semaphore_removability_semaphore->lock();
  if (semaphore_removability_semaphore->countUnsafe() > 0) {
    semaphore_removability_semaphore->incUnsafe();
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
    // auto wr = ProjectileInsertWrap(std::move(projectile));
    // projectile_inserter = std::move(wr);
    phom.prependObject(std::move(projectile));
    unsigned cooldown = (unsigned)std::rand() % (hi_cooldown - lo_cooldown);
    cooldown += lo_cooldown;
    setUnready();
    return std::async(
      cooldown_fn,
      std::ref(*this), cooldown,
      availability_semaphore, semaphore_removability_semaphore
    );
  } else {
    return Basic::fire(from, to, base_velocity); // no-op
  }
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
  // bool res = ready_semaphore.count() == 0;
  // return res;
  return ready_semaphore.count() == 0;
}

// Basic::Lock Basic::read() {
//   return Lock(mx);
// }

// Basic::Lock Basic::write() {
//   return Lock(mx);
// }

Basic& Basic::setUnready() {
  if (isReady()) {
    ready_semaphore.inc();
  }
  return *this;
}

Basic& Basic::setReady() {
  if (!isReady()) {
    ready_semaphore.dec();
  }
  return *this;
}

} // weapon



void ArmedObject::fire(Point2d cursor) {
  if (weapon && weapon->isReady()) {
    cooldowner = weapon->fire(pos, cursor, Point2d{.y = (int)y_vel, .x = (int)x_vel});
  }
}


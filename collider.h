#pragma once
#ifndef COLLIDER_H_SENTRY
#define COLLIDER_H_SENTRY

#include "geometry.h"
#include "sprite.h"
#include <fstream>
#include <string>
#include <vector>
#include <stdint.h>

using CollisionMask = uint16_t;

class Collider {
  std::unique_ptr<CollisionShape> collision_shape;
  CollisionMask hit_mask = 0;
  CollisionMask receive_mask = 0;
public:
  Collider(
    std::unique_ptr<CollisionShape>&& cs, CollisionMask hm=0, CollisionMask rm=0
  ) : collision_shape(std::move(cs)), hit_mask(hm), receive_mask(rm) {}
  // TODO safety
  void setHitOn(unsigned i) { hit_mask |= (1 << i); }
  void setHitOff(unsigned i) { hit_mask &= ~(1 << i); }
  bool checkHit(unsigned i) { return hit_mask & (1 << i); }
  void setReceiveOn(unsigned i) { receive_mask |= (1 << i); }
  void setReceiveOff(unsigned i) { receive_mask &= ~(1 << i); }
  bool checkReceive(unsigned i) { return receive_mask & (1 << i); }
  struct Callback {
    virtual void operator()(Collider& hitter, Collider& receiver, int depth=0) = 0;
    virtual ~Callback() {}
  };
  void collide(Collider& receiver, Callback& cb) {
    if (hit_mask & receiver.receive_mask) cb(*this, receiver);
  }
  enum struct Mask : unsigned {
    player=0,
  };
};

class CollisionObject : public SpriteObject {
  std::vector<std::unique_ptr<Collider>> colliders;
public:
  CollisionObject() = default;
  CollisionObject(Point2d pos, float rot, SpriteRef sprite, std::optional<Box2d> ptl = std::nullopt)
    : SpriteObject(pos, rot, std::move(sprite), std::move(ptl)) {}
  void add(std::unique_ptr<Collider> collider) { colliders.push_back(std::move(collider)); }
  const Collider& operator[](unsigned i) const { return *colliders[i]; }
  Collider& operator[](unsigned i) { return *colliders[i]; }
  using ConstIterator = std::vector<std::unique_ptr<Collider>>::const_iterator;
  using Iterator = std::vector<std::unique_ptr<Collider>>::iterator;
  ConstIterator begin() const { return colliders.begin(); }
  ConstIterator end() const { return colliders.end(); }
  Iterator begin() { return colliders.begin(); }
  Iterator end() { return colliders.end(); }

  class Error {};
  class FOError: public Error {
    const char *file;
  public:
    FOError(const char *file): file(file) {}
    const char *what() const { return file; }
  };
  class FRError: public Error {
    const char *file;
  public:
    FRError(const char *file): file(file) {}
    const char *what() const { return file; }
  };
  static CollisionObject readFromFile(const char *file)
  {
    std::ifstream fs(file);
    if (fs) {
      CollisionObject co;
      std::string type;
      // while (std::getline(fs, type)) {
      while (fs) { // TODO will this do the trick??
        co.add(std::make_unique<Collider>(collision_shape::readFromStream(fs)));
        fs >> std::ws;
      }
      return co;
    } else {
      throw FOError(file);
    }
  }
};

#endif // COLLIDER_H_SENTRY

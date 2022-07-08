#pragma once
#ifndef COLLIDER_H_SENTRY
#define COLLIDER_H_SENTRY

#include "geometry.h"
#include "sprite.h"
#include <fstream>
#include <string>
#include <vector>
#include <stdint.h>

// using CollisionMask = uint16_t;
class CollisionMask {
public:
  using BaseInt = uint16_t;
private:
  BaseInt val;
public:
  // hitMask(v) and receiveMask(v) should be preferred
  CollisionMask(BaseInt v): val(v) {}
  CollisionMask() = default;
  CollisionMask(const CollisionMask&) = default;
  CollisionMask& operator=(const CollisionMask&) = default;
  CollisionMask& operator=(BaseInt v) { return *this = CollisionMask(v); }
  operator BaseInt() const { return val; }
  CollisionMask& operator|=(BaseInt v) { val |= v; return *this; }
  CollisionMask& operator&=(BaseInt v) { val &= v; return *this; }
  CollisionMask& operator|=(CollisionMask cm) { val |= cm; return *this; }
  CollisionMask& operator&=(CollisionMask cm) { val &= cm; return *this; }
};

class CollisionObject;
class Collider {
  std::unique_ptr<CollisionShape> collision_shape;
  CollisionMask hit_mask = 0;
  CollisionMask receive_mask = 0;
public:
  Collider(
    std::unique_ptr<CollisionShape>&& cs, CollisionMask hm=0, CollisionMask rm=0
  ) : collision_shape(std::move(cs)), hit_mask(hm), receive_mask(rm) {}
  Collider(Collider&&) = default;
  Collider(const Collider& other)
    : collision_shape(other.collision_shape->copy())
    , hit_mask(other.hit_mask), receive_mask(other.receive_mask) {}
  Collider transformed(const TransformChain& chain) const {
    return Collider(collision_shape->transformed(chain), hit_mask, receive_mask);
  }
  // TODO safety
  void setHitOn(unsigned i) { hit_mask |= (1 << i); }
  void setHitOff(unsigned i) { hit_mask &= ~(1 << i); }
  bool checkHit(unsigned i) const { return hit_mask & (1 << i); }
  void setReceiveOn(unsigned i) { receive_mask |= (1 << i); }
  void setReceiveOff(unsigned i) { receive_mask &= ~(1 << i); }
  bool checkReceive(unsigned i) const { return receive_mask & (1 << i); }
  void setHit(CollisionMask m) { hit_mask = m; }
  void setReceive(CollisionMask m) { receive_mask = m; }
  struct Callback {
    virtual void operator()(Collider& hitter, Collider& receiver, int depth=0) {
      (void)hitter; (void)receiver; (void)depth;
    }
    virtual ~Callback() {}
  };
  void collide(Collider& receiver, Callback& cb) {
    if (hit_mask & receiver.receive_mask) cb(*this, receiver);
  }
  enum struct MaskIdx : unsigned {
    player=0,
    enemy=1,
  };
  enum struct Mask : CollisionMask::BaseInt {
    player = 1 << static_cast<unsigned>(MaskIdx::player),
    enemy = 1 << static_cast<unsigned>(MaskIdx::enemy),
  };
  friend CollisionObject;
#ifdef DEBUG
  void draw(uint32_t *buffer, unsigned screen_h, unsigned screen_w) {
    collision_shape->draw(buffer, screen_h, screen_w);
  }
#endif
};

CollisionMask hitMask(Collider::Mask v);
CollisionMask receiveMask(Collider::Mask v);

class CollisionObject : public SpriteObject {
protected:
  std::vector<Collider> orig; // to keep original objects
  std::vector<Collider> transformed; // for actual collision
public:
  CollisionObject() = default;
  CollisionObject(Point2d pos, float rot, SpriteRef sprite, std::optional<Box2d> ptl = std::nullopt)
    : SpriteObject(pos, rot, std::move(sprite), std::move(ptl)) {}
  void addOrig(Collider collider) { orig.push_back(std::move(collider)); }
  // const Collider& operator[](unsigned i) const { return *transformed[i]; }
  // Collider& operator[](unsigned i) { return *colliders[i]; }
  using ConstIterator = std::vector<Collider>::const_iterator;
  using Iterator = std::vector<Collider>::iterator;
  ConstIterator origBegin() const { return orig.begin(); }
  ConstIterator origEnd() const { return orig.end(); }
  Iterator origBegin() { return orig.begin(); }
  Iterator origEnd() { return orig.end(); }
  ConstIterator transformedBegin() const { return transformed.begin(); }
  ConstIterator transformedEnd() const { return transformed.end(); }
  Iterator transformedBegin() { return transformed.begin(); }
  Iterator transformedEnd() { return transformed.end(); }

  void act(float dt) override; // update colliders

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
        co.addOrig(Collider(collision_shape::readFromStream(fs)));
        fs >> std::ws;
      }
      return co;
    } else {
      throw FOError(file);
    }
  }

#ifdef DEBUG
  void draw(uint32_t *buffer, unsigned screen_h, unsigned screen_w) override;
#endif
};

#endif // COLLIDER_H_SENTRY

#include "collider.h"


void CollisionObject::act(float dt) {
  SpriteObject::act(dt);
  transformed.clear();
  transformed.reserve(orig.size());
  TransformChain chain;
  chain.append(std::make_unique<Rotation>(rot));
  chain.append(std::make_unique<Translation>(pos));
  for (const auto& orig_ptr : orig) {
    transformed.push_back(orig_ptr.transformed(chain));
  }
}

CollisionMask hitMask(Collider::Mask v) {
  return CollisionMask(static_cast<CollisionMask::BaseInt>(v));
}
CollisionMask receiveMask(Collider::Mask v) {
  return CollisionMask(static_cast<CollisionMask::BaseInt>(v));
}

void CollisionObject::collide(CollisionObject& receiver, Callback& cb) {
  for (auto hiter = transformedBegin(); hiter != transformedEnd(); ++hiter) {
    for (auto riter = receiver.transformedBegin(); riter != receiver.transformedEnd(); ++riter) {
      if (hiter->hit_mask & riter->receive_mask) {
        if (hiter->collision_shape->collides(*riter->collision_shape)) {
          cb(*this, receiver);
          goto BREAK;
        }
      }
    }
  }
BREAK:;
}

#ifdef DEBUG
void CollisionObject::draw(uint32_t *buffer, unsigned screen_h, unsigned screen_w) {
  SpriteObject::draw(buffer, screen_h, screen_w);
  for (auto iter = transformedBegin(); iter != transformedEnd(); ++iter) {
    iter->draw(buffer, screen_h, screen_w);
  }
}
#endif

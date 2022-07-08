#pragma once
#ifndef PLAYER_H_SENTRY
#define PLAYER_H_SENTRY

#include "weapon.h"

class Player : public ArmedObject {
  Point2d move(float dt);

public:
  Player(Point2d pos, float rot, SpriteRef sprite, Cooldowner cdr, std::optional<Box2d> ptl = std::nullopt);
  void act(float dt) override;
}; // Player


#endif // PLAYER_H_SENTRY

#include "player.h"
#include "Engine.h" // can I just do it like this???

enum {
  player_linear_accel = 14142,
  player_linear_viscousity = 10,
};


Point2d Player::move(float dt) {
  // suicide
  if (is_key_pressed(VK_SPACE)) {
    pos.y = -300;
    pos.x = -300;
    y_frac = -300;
    x_frac = -300;
    return Point2d{.y=0, .x=0};
  }

  x_acc = 0;
  y_acc = 0;
  if (is_key_pressed(VK_RIGHT)) {
    x_acc = player_linear_accel;
  }
  if (is_key_pressed(VK_LEFT)) {
    x_acc = -player_linear_accel;
  }
  if (is_key_pressed(VK_DOWN)) {
    y_acc = player_linear_accel;
  }
  if (is_key_pressed(VK_UP)) {
    y_acc = -player_linear_accel;
  }
  if (x_acc != 0 && y_acc != 0) {
    x_acc /= 1.4142;
    y_acc /= 1.4142;
  }
  x_acc -= player_linear_viscousity * x_vel;
  y_acc -= player_linear_viscousity * y_vel;
  PhysicalObject::act(dt);
  // TODO round??
  pos.x = x_frac;
  pos.y = y_frac;

  Point2d cursor{
    .y = get_cursor_y(),
    .x = get_cursor_x(),
  };
  rot = (cursor - pos).angle(Point2d{.y = -1, .x = 0});

  return cursor;
}

void Player::act(float dt) {
  Point2d cursor = move(dt);
  if (is_mouse_button_pressed(0)) {
    // (void)cursor;
    fire(cursor);
  }
}

Player::Player(Point2d pos, float rot, SpriteRef sprite, Cooldowner cdr, std::optional<Box2d> ptl)
  : ArmedObject(pos, rot, std::move(sprite), cdr, std::move(ptl))
{
  for (std::unique_ptr<Collider>& col : static_cast<CollisionObject&>(*this)) {
    col->setReceiveOn(static_cast<unsigned>(Collider::Mask::player));
  }
}

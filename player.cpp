#include "player.h"
#include "Engine.h" // can I just do it like this???

enum {
  player_linear_accel = 10000,
  player_linear_viscousity = 10,
};

#ifdef DEBUG
uint32_t green = 0 ^ (255 << 24) ^ (255 << 8);
#endif

void Player::draw(uint32_t *buffer, unsigned screen_h, unsigned screen_w) {
  Box2d sprite_box0{
    .lt = Point2d{.y = 0, .x=0,},
    .rb = Point2d{.y = (int)sprite.getHeight(), .x = (int)sprite.getWidth(),}
  };

  Rectangle rect(sprite_box0);
  TransformChain chain;
  chain.append(std::make_unique<Translation>(rect.translate_(-0.5*sprite_box0.rb)));
  chain.append(std::make_unique<Rotation>(rect.rotate_(rot)));
  chain.append(std::make_unique<Translation>(rect.translate_(pos)));

  Box2d bounding_box = boundingBox2d(rect);
  Box2d screen_box = Box2d{
    .lt = Point2d{.y = 0, .x = 0},
    .rb = Point2d{.y = (int)screen_h, .x = (int)screen_w},
  };
  bounding_box.intersect_(screen_box);
  
  for (int i = bounding_box.lt.y; i < bounding_box.rb.y; ++i) {
    for (int j = bounding_box.lt.x; j < bounding_box.rb.x; ++j) {
      Point2d point = Point2d{.y=i, .x=j};
      if (rect.contains(point)) {
        point = chain.backward(point);

        // finally
        // TODO actual color addition??
        buffer[i*screen_w + j] = sprite(point.y, point.x).toBGRA();
        // buffer[100 + 100*screen_w + i*screen_w + j] = sprite(i, j).toBGRA();
      }
    }
  }

#ifdef DEBUG
  for (int i = bounding_box.lt.y; i < bounding_box.rb.y; ++i) {
    buffer[i*screen_w + bounding_box.lt.x] = green;
    buffer[i*screen_w + bounding_box.rb.x-1] = green;
  }
  for (int j = bounding_box.lt.x; j < bounding_box.rb.x; ++j) {
    buffer[bounding_box.lt.y*screen_w + j] = green;
    buffer[(bounding_box.rb.y-1)*screen_w + j] = green;
  }
#endif
}

void Player::act(float dt) {
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
}

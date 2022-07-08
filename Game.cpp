#include "Engine.h"
#include <stdlib.h>
#include <memory.h>

#include <stdio.h>

#include "physical_object.h"
#include "sprite.h"
#include "player.h"
#include "enemy.h"
#include "log.h"
#include <list>
#include <chrono>
#include <vector>
#include <memory>
//
//  You are free to modify this file
//

//  is_key_pressed(int button_vk_code) - check if a key is pressed,
//                                       use keycodes (VK_SPACE, VK_RIGHT, VK_LEFT, VK_UP, VK_DOWN, VK_RETURN)
//
//  get_cursor_x(), get_cursor_y() - get mouse cursor position
//  is_mouse_button_pressed(int button) - check if mouse button is pressed (0 - left button, 1 - right button)
//  schedule_quit_game() - quit game after act()

const char *player_sprite_file = "sprites/player.b";
const char *sphere_sprite_file = "sprites/sphere.b";

Armory armory;

PhysicalObjectManager phom;
// std::list<std::unique_ptr<PhysicalObject>> physical_objects;

// initialize game data in this function
void initialize()
{
  try {
    // armory.load(std::insert_iterator<decltype(physical_objects)>(physical_objects, physical_objects.begin()));
    // armory.load(phom.objectPrepender());
    armory.load(&phom);
  } catch (const Sprite::FOError& err) {
    log("failed to open ", err.what(), "\n");
    throw;
  } catch (const Sprite::FRError& err) {
    log("failed to read ", err.what(), "\n");
    throw;
  }

  {
    SpriteRef player_sprite(0,0);
    try {
      *player_sprite = Sprite::fromBinaryFile(player_sprite_file);
    } catch (const Sprite::FOError& err) {
      log("failed to open ", err.what(), "\n");
      throw;
    } catch (const Sprite::FRError& err) {
      log("failed to read ", err.what(), "\n");
      throw;
    }
    std::unique_ptr<Player> player = std::make_unique<Player>(
        Point2d{.y = SCREEN_HEIGHT/2, .x = SCREEN_WIDTH/2}, 0, std::move(player_sprite),
        phom.futureAppender(),
        Box2d{
          .lt=Point2d{.y=0, .x=0},
          .rb=Point2d{.y=SCREEN_HEIGHT, .x=SCREEN_WIDTH},
        }
    );
    player->arm(armory[0], hitMask(Collider::Mask::enemy), receiveMask(Collider::Mask::player));
    auto player_handle = phom.appendSemaphoredObject(std::move(player));
    auto sm = *player_handle->getSemaphore();
    static_cast<ArmedObject&>(**player_handle).weaponSetUtilSemaphores(
        std::make_pair(&sm->payload_semaphore, &sm->can_remove_semaphore)
    );
  }

  {
    SpriteRef sphere_sprite(0,0);
    try {
      *sphere_sprite = Sprite::fromBinaryFile(sphere_sprite_file);
    } catch (const Sprite::FOError& err) {
      log("failed to open ", err.what(), "\n");
      throw;
    } catch (const Sprite::FRError& err) {
      log("failed to read ", err.what(), "\n");
      throw;
    }
    std::unique_ptr<enemy::Sphere> sphere = std::make_unique<enemy::Sphere>(
        Point2d{.y = 50, .x = 50}, 0, std::move(sphere_sprite),
        phom.futureAppender(),
        Box2d{
          .lt=Point2d{.y=0, .x=0},
          .rb=Point2d{.y=SCREEN_HEIGHT, .x=SCREEN_WIDTH},
        }
    );
    sphere->arm(armory[2], hitMask(Collider::Mask::player), receiveMask(Collider::Mask::enemy));
    auto sphere_handle = phom.appendSemaphoredObject(std::move(sphere));
    auto sm = *sphere_handle->getSemaphore();
    static_cast<ArmedObject&>(**sphere_handle).weaponSetUtilSemaphores(
      std::make_pair(&sm->payload_semaphore, &sm->can_remove_semaphore)
    );
  }
}

// this function is called to update game data,
// dt - time elapsed since the previous update (in seconds)
const Box2d existential_box = Box2d{
  .lt = Point2d{.y = -200, .x = -200},
  .rb = Point2d{.y = SCREEN_HEIGHT + 200, .x = SCREEN_WIDTH + 200},
};
void act(float dt)
{
  if (is_key_pressed(VK_ESCAPE))
    schedule_quit_game();

  // log(phom.objectCount(), " objects ");
  // log(phom.semaphoreCount(), " semaphores ");
  // log(phom.futureCount(), " futures\n");
  {
    auto iter = phom.objectsBegin();
    while (iter != phom.objectsEnd()) {
      PhysicalObject& obj = **iter;
      if (!obj.isInsideBox(existential_box)) {
        iter = phom.eraseObject(iter);
        continue;
      } else {
        ++iter;
        continue;
      }
    }
  }

  {
    auto iter = phom.semaphoresBegin();
    while (iter != phom.semaphoresEnd()) {
      auto switch_erasure_result = phom.eraseSemaphore(iter);
      if (switch_erasure_result) {
        iter = *switch_erasure_result;
      } else {
        ++iter;
      }
    }
  }

  for (auto iter = phom.futuresBegin(); iter != phom.futuresEnd();) {
    if (iter->wait_for(std::chrono::seconds(0)) == std::future_status::ready) {
      iter = phom.eraseFuture(iter);
    } else {
      ++iter;
    }
  }

  for (auto iter = phom.objectsBegin(); iter != phom.objectsEnd(); ++iter) {
    PhysicalObject& obj = **iter;
    obj.act(dt);
  }

}

// fill buffer in this function
// uint32_t buffer[SCREEN_HEIGHT][SCREEN_WIDTH] - is an array of 32-bit colors (8 bits per R, G, B)
void draw()
{
  // clear backbuffer
  memset(buffer, 0, SCREEN_HEIGHT * SCREEN_WIDTH * sizeof(uint32_t));

  for (auto iter = phom.objectsBegin(); iter != phom.objectsEnd(); ++iter) {
    PhysicalObject& obj = **iter;
    // undefined behaviour
    obj.draw((uint32_t*)buffer, SCREEN_HEIGHT, SCREEN_WIDTH);
  }

}

// free game data in this function
void finalize()
{
}


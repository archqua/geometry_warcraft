#include "Engine.h"
#include <stdlib.h>
#include <memory.h>

#include <stdio.h>

#include "physical_object.h"
#include "sprite.h"
#include "player.h"
#include "enemy.h"
#include "rand_mx.h"
#include "log.h"
#include <list>
#include <unistd.h>
#include <time.h>
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

// globals begin
const char *player_sprite_file = "sprites/player.b";
SpriteRef player_sprite(0,0);
const char *sphere_sprite_file = "sprites/sphere.b";
SpriteRef sphere_sprite(0,0);
const char *welcome_sprite_file = "sprites/welcome.b";
SpriteRef welcome_sprite(0,0);

uint32_t welcome_buffer[SCREEN_HEIGHT][SCREEN_WIDTH];
uint32_t *initializer_buffer = nullptr;

Armory armory;
PhysicalObjectManager phom;

Point2d enemy_spawn_locations[] {
  Point2d{.y=0                , .x=0               },
  Point2d{.y=  SCREEN_HEIGHT/6, .x=0               },
  Point2d{.y=2*SCREEN_HEIGHT/6, .x=0               },
  Point2d{.y=3*SCREEN_HEIGHT/6, .x=0               },
  Point2d{.y=4*SCREEN_HEIGHT/6, .x=0               },
  Point2d{.y=5*SCREEN_HEIGHT/6, .x=0               },
  Point2d{.y=  SCREEN_HEIGHT  , .x=0               },
  Point2d{.y=  SCREEN_HEIGHT  , .x=  SCREEN_WIDTH/8},
  Point2d{.y=  SCREEN_HEIGHT  , .x=2*SCREEN_WIDTH/8},
  Point2d{.y=  SCREEN_HEIGHT  , .x=3*SCREEN_WIDTH/8},
  Point2d{.y=  SCREEN_HEIGHT  , .x=4*SCREEN_WIDTH/8},
  Point2d{.y=  SCREEN_HEIGHT  , .x=5*SCREEN_WIDTH/8},
  Point2d{.y=  SCREEN_HEIGHT  , .x=6*SCREEN_WIDTH/8},
  Point2d{.y=  SCREEN_HEIGHT  , .x=7*SCREEN_WIDTH/8},
  Point2d{.y=  SCREEN_HEIGHT  , .x=  SCREEN_WIDTH  },
  Point2d{.y=5*SCREEN_HEIGHT/6, .x=  SCREEN_WIDTH  },
  Point2d{.y=4*SCREEN_HEIGHT/6, .x=  SCREEN_WIDTH  },
  Point2d{.y=3*SCREEN_HEIGHT/6, .x=  SCREEN_WIDTH  },
  Point2d{.y=2*SCREEN_HEIGHT/6, .x=  SCREEN_WIDTH  },
  Point2d{.y=  SCREEN_HEIGHT/6, .x=  SCREEN_WIDTH  },
  Point2d{.y=0                , .x=  SCREEN_WIDTH  },
  Point2d{.y=0                , .x=7*SCREEN_WIDTH/8},
  Point2d{.y=0                , .x=6*SCREEN_WIDTH/8},
  Point2d{.y=0                , .x=5*SCREEN_WIDTH/8},
  Point2d{.y=0                , .x=4*SCREEN_WIDTH/8},
  Point2d{.y=0                , .x=3*SCREEN_WIDTH/8},
  Point2d{.y=0                , .x=2*SCREEN_WIDTH/8},
  Point2d{.y=0                , .x=  SCREEN_WIDTH/8},
};

Semaphore enemy_spawn_semaphore(1);
Semaphore welcome_usage_semaphore(0);
std::future<void> enemy_spawn_future;

// globals end

void loadSprite(SpriteRef& sprite_ref, const char *sprite_file)
{
  try {
    *sprite_ref = Sprite::fromBinaryFile(sprite_file);
  } catch (const Sprite::FOError& err) {
    log("failed to open sprite file: ", err.what(), "\n");
    throw;
  } catch (const Sprite::FRError& err) {
    log("failed to read sprite file: ", err.what(), "\n");
    throw;
  }
}

void spawnPlayer(Point2d loc = Point2d{.y=SCREEN_HEIGHT/2, .x=SCREEN_WIDTH/2})
{
  std::unique_ptr<Player> player = std::make_unique<Player>(
      loc, 0, player_sprite,
      phom.futureAppender(),
      Box2d{
        .lt=Point2d{.y=0, .x=0},
        .rb=Point2d{.y=SCREEN_HEIGHT, .x=SCREEN_WIDTH},
      }
  );
  player->arm(armory[0], hitMask(Collider::Mask::enemy), receiveMask(Collider::Mask::player));
  auto player_handle = phom.appendSemaphoredCollider(std::move(player));
  auto sm = *player_handle->getSemaphore();
  static_cast<ArmedObject&>(**player_handle).weaponSetUtilSemaphores(
      std::make_pair(&sm->payload_semaphore, &sm->can_remove_semaphore)
  );
}

void spawnSphere(Point2d loc)
{
  std::unique_ptr<enemy::Sphere> sphere = std::make_unique<enemy::Sphere>(
      loc, 0, sphere_sprite,
      phom.futureAppender(),
      Box2d{
        .lt=Point2d{.y=0, .x=0},
        .rb=Point2d{.y=SCREEN_HEIGHT, .x=SCREEN_WIDTH},
      }
  );
  sphere->arm(armory[2], hitMask(Collider::Mask::player), receiveMask(Collider::Mask::enemy));
  auto sphere_handle = phom.appendSemaphoredCollider(std::move(sphere));
  auto sm = *sphere_handle->getSemaphore();
  static_cast<ArmedObject&>(**sphere_handle).weaponSetUtilSemaphores(
    std::make_pair(&sm->payload_semaphore, &sm->can_remove_semaphore)
  );
}

void spawnEnemies(unsigned init_delay /* ms */ = 500, unsigned interval /* ms */ = 1500)
{
  unsigned seconds = interval / 1000;
  unsigned useconds = 1000 * (interval % 1000);
  unsigned pos_idx = 0;
  {
    std::unique_lock rand_lock(rand_mx);
    srand(time(NULL));
  }
  unsigned iseconds = init_delay / 1000;
  unsigned iuseconds = 1000 * (init_delay % 1000);
  sleep(iseconds);
  usleep(iuseconds);
  while (enemy_spawn_semaphore.count()) {
    spawnSphere(enemy_spawn_locations[pos_idx]);
    {
      std::unique_lock rand_lock(rand_mx);
      pos_idx = rand() % (sizeof(enemy_spawn_locations)/sizeof(Point2d));
    }
    sleep(seconds);
    usleep(useconds);
  }
}

void showWelcomeAndSpawnEnemies(
    unsigned welcome_dur /* ms */ = 2000,
    unsigned init_delay /* ms */ = 500, unsigned interval /* ms */ = 1500
)
{
  std::unique_ptr<SpriteObject> welcome = std::make_unique<SpriteObject>(
      Point2d{.y=SCREEN_HEIGHT/2, .x=SCREEN_WIDTH/2}, 0, welcome_sprite
  );
  // undefined behaviour
  welcome->draw((uint32_t*)welcome_buffer, SCREEN_HEIGHT, SCREEN_WIDTH);
  welcome_usage_semaphore.inc();
  // undefined behaviour
  initializer_buffer = (uint32_t*)welcome_buffer;
  welcome_usage_semaphore.dec();
  unsigned seconds = welcome_dur / 1000;
  unsigned useconds = 1000 * (welcome_dur % 1000);
  sleep(seconds);
  usleep(useconds);
  welcome_usage_semaphore.inc();
  initializer_buffer = nullptr;
  welcome_usage_semaphore.dec();
  spawnEnemies(init_delay, interval);
}

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

  loadSprite(player_sprite, player_sprite_file);
  loadSprite(sphere_sprite, sphere_sprite_file);
  loadSprite(welcome_sprite, welcome_sprite_file);

  spawnPlayer();
  // enemy_spawn_future = std::async(spawnEnemies, 500, 1500);
  enemy_spawn_future = std::async(showWelcomeAndSpawnEnemies, 2000, 500, 1500);
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

  if (is_key_pressed(VK_SPACE))
    return;
  // log(phom.objectCount(), " objects ");
  // log(phom.semaphoreCount(), " semaphores ");
  // log(phom.futureCount(), " futures ");
  // log(phom.colliderCount(), " colliders\n");

  // erase nonexistent
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
    auto iter = phom.collidersBegin();
    while (iter != phom.collidersEnd()) {
      CollisionObject& obj = **iter;
      if (!obj.isInsideBox(existential_box)) {
        iter = phom.eraseCollider(iter);
        continue;
      } else {
        ++iter;
        continue;
      }
    }
  }

  // collide
  {
    auto hiter = phom.collidersBegin();
    while (hiter != phom.collidersEnd()) {
      CollisionObject& obj = **hiter;
      if (!obj.isInsideBox(existential_box)) {
        hiter = phom.eraseCollider(hiter);
        continue;
      } else {
        auto riter = phom.collidersBegin();
        while (riter != phom.collidersEnd()) {
          if (hiter == riter) {
            ++riter;
            continue;
          } else {
            bool detected_collision = false;
            PhysicalObjectManager::MutualRemovalCallback cb(phom, hiter, riter, detected_collision);
            (*hiter)->collide(**riter, cb);
            if (detected_collision) {
              goto HITER_CONTINUE;
            } else {
              ++riter;
              continue;
            }
          }
        }
        ++hiter;
HITER_CONTINUE:
        continue;
      }
    }
  }

  // clean up semaphores
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

  // clean up futures
  for (auto iter = phom.futuresBegin(); iter != phom.futuresEnd();) {
    if (iter->wait_for(std::chrono::seconds(0)) == std::future_status::ready) {
      iter = phom.eraseFuture(iter);
    } else {
      ++iter;
    }
  }

  // call act
  for (auto iter = phom.objectsBegin(); iter != phom.objectsEnd(); ++iter) {
    PhysicalObject& obj = **iter;
    obj.act(dt);
  }
  for (auto iter = phom.collidersBegin(); iter != phom.collidersEnd(); ++iter) {
    CollisionObject& obj = **iter;
    obj.act(dt);
  }

}

// fill buffer in this function
// uint32_t buffer[SCREEN_HEIGHT][SCREEN_WIDTH] - is an array of 32-bit colors (8 bits per R, G, B)
void draw()
{
  {
  auto l = welcome_usage_semaphore.lock();
  if (welcome_usage_semaphore.countUnsafe()) {
    log("no draw at welcome usage ", welcome_usage_semaphore.countUnsafe());
    return;
  }
  // clear backbuffer
  if (initializer_buffer) {
    memcpy(buffer, initializer_buffer, SCREEN_HEIGHT * SCREEN_WIDTH * sizeof(uint32_t));
  } else {
    memset(buffer, 0, SCREEN_HEIGHT * SCREEN_WIDTH * sizeof(uint32_t));
  }
  }

  for (auto iter = phom.objectsBegin(); iter != phom.objectsEnd(); ++iter) {
    PhysicalObject& obj = **iter;
    // undefined behaviour
    obj.draw((uint32_t*)buffer, SCREEN_HEIGHT, SCREEN_WIDTH);
  }
  for (auto iter = phom.collidersBegin(); iter != phom.collidersEnd(); ++iter) {
    CollisionObject& obj = **iter;
    // undefined behaviour
    obj.draw((uint32_t*)buffer, SCREEN_HEIGHT, SCREEN_WIDTH);
  }

}

// free game data in this function
void finalize()
{
  enemy_spawn_semaphore.dec();
  // w\o get screen is closed immediately
  // and it feels less like game just froze
  // enemy_spawn_future.get();
}


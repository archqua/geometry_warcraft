#include "Engine.h"
#include <stdlib.h>
#include <memory.h>

#include <stdio.h>

#include "physical_object.h"
#include "sprite.h"
#include "player.h"
#include "log.h"
#include <list>
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

std::list<std::unique_ptr<PhysicalObject>> physical_objects;


// initialize game data in this function
void initialize()
{
  Sprite player_sprite(0,0);
  try {
    player_sprite = Sprite::fromBinaryFile(player_sprite_file);
  } catch (const Sprite::FOError& err) {
    log("failed to open ");
    log(err.what());
    log("\n");
    throw;
  } catch (const Sprite::FRError& err) {
    log("failed to read ");
    log(err.what());
    log("\n");
    throw;
  }
  std::unique_ptr<PhysicalObject> player = std::make_unique<Player>(Point2d{.y = 0, .x = 0}, 0, std::move(player_sprite));
  physical_objects.push_back(std::move(player));
}

// this function is called to update game data,
// dt - time elapsed since the previous update (in seconds)
void act(float dt)
{
  if (is_key_pressed(VK_ESCAPE))
    schedule_quit_game();

  for (auto& obj : physical_objects) {
    obj->act(dt);
  }

}

// fill buffer in this function
// uint32_t buffer[SCREEN_HEIGHT][SCREEN_WIDTH] - is an array of 32-bit colors (8 bits per R, G, B)
void draw()
{
  // clear backbuffer
  memset(buffer, 0, SCREEN_HEIGHT * SCREEN_WIDTH * sizeof(uint32_t));

  for (auto& obj : physical_objects) {
    // TODO find sane solution
    // obj->draw(static_cast<uint32_t*>(buffer), SCREEN_HEIGHT, SCREEN_WIDTH); // invalid static_cast
    // this one is insane
    obj->draw((uint32_t*)buffer, SCREEN_HEIGHT, SCREEN_WIDTH);
  }

}

// free game data in this function
void finalize()
{
}


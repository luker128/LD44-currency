#include <GLES3/gl3.h>
#include <cstdlib>
#include <string>
#include <vector>
#include <sstream>
#include <iostream>
#include "main.h"
#include "gfx.h"



class Game {

    Game(const Game&) = delete;

  public:

    Game() : coinImage("data/coin.png", 32, 32) {
    }

    bool gameLoop() {
      update();
      draw();
      return true;
    }

    void keyPress(bool pressed, unsigned char key, unsigned short code) {
      if (key == 13) { // enter
      }
      if (key == 32) { // space
        key_jump = pressed;
      }
      if (key == 79 || code == 39) {
        key_right = pressed;
      } else if (key == 80 || code == 37) {
        key_left = pressed;
      } else if (key == 81 || code == 40) {
        key_down = pressed;
      } else if (key == 82 || code == 38) {
        key_up = pressed;
      }
    }

  private:

    void update() {
      time++;
    }

    void draw() {
      float angle = time / 30.0;
      coinImage.drawSpriteRotated(100,100,0, angle);
    }

    int time = 0;

    SpriteSheet coinImage;
    bool key_jump = false;
    bool key_down = false;
    bool key_up = false;
    bool key_left = false;
    bool key_right = false;

};






Game* game = nullptr;


void key_press(bool pressed, unsigned char key, unsigned short code) {
  game->keyPress(pressed, key, code);
}

void mouse_button(bool pressed, int button, int x, int y ) {
}

bool gameLoop() {
  return game->gameLoop();
}

void gameInit() {
  createWindow(960, 540, "Currency");
  glViewport(0, 0, screen_w, screen_h);
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  game = new Game();
}

void gameCleanup() {
  delete game;
}


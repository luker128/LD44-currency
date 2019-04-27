#include <GLES3/gl3.h>
#include <cstdlib>
#include <string>
#include <vector>
#include <sstream>
#include <iostream>
#include "main.h"
#include "gfx.h"

  Image* coinImage;
  bool key_jump = false;
  bool key_down = false;
  bool key_up = false;
  bool key_left = false;
  bool key_right = false;


void draw() {
}

void update() {
}

void key_press(bool pressed, unsigned char key, unsigned short code) {
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
void mouse_button(bool pressed, int button, int x, int y ) {
}

void scrollDown();
bool showIntro();
bool showOutro();

bool gameLoop() {
  update();
  draw();
  return true;
}

void gameInit() {
  createWindow(960, 540, "Currency");
  glViewport(0, 0, screen_w, screen_h);
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  coinImage = new Image("data/coin.png");
}

void gameCleanup() {
  delete coinImage;
}


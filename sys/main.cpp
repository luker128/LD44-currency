#include <GLES3/gl3.h>
#include <iostream>

#include "main.h"

int screen_w = 600;
int screen_h = 450;
int mouse_x = 0;
int mouse_y = 0;
bool mouse_left = false;
bool mouse_right = false;
int joy_x = 0;
int joy_y = 0;

void startMainLoop();
bool processInput();

int main(int, char**) {
  gameInit();
  startMainLoop();
  gameCleanup();
  return 0;
}

bool processFrame() {
  if (!processInput()) {
    return false;
  }
  return gameLoop();
}



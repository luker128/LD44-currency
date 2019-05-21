#ifndef __EMSCRIPTEN__

#include <GLES3/gl3.h>
#include <iostream>
#include <sstream>

#include "main.h"
#include <SDL2/SDL.h>

SDL_GameController *controller = NULL;
bool processFrame();

bool keys[SDL_NUM_SCANCODES];
bool processInput() {
  SDL_Event event;
  while (SDL_PollEvent(&event)) {
    switch(event.type) {
      case SDL_QUIT: {
        return false;
      }
      case SDL_KEYDOWN: {
        if (event.key.keysym.sym == SDLK_ESCAPE) {
          return false;
        }
        keys[(unsigned char)event.key.keysym.sym] = true;
        key_press(true, (unsigned char) event.key.keysym.sym, event.key.keysym.scancode);
        break;
      }
      case SDL_KEYUP: {
        keys[(unsigned char)event.key.keysym.sym] = false;
        key_press(false, (unsigned char) event.key.keysym.sym, event.key.keysym.scancode);
        break;
      }
      case SDL_MOUSEMOTION: {
        mouse_x = event.motion.x;
        mouse_y = event.motion.y;
        break;
      }
      case SDL_MOUSEWHEEL: {
        std::cout << "Mouse wheel " << event.wheel.y << std::endl;
        mouse_wheel(event.wheel.y);
      }
      case SDL_MOUSEBUTTONDOWN: {
        if (event.button.button == 1) {
          mouse_left = true;
        }
        if (event.button.button == 3) {
          mouse_right = true;
        }
        mouse_button(true, event.button.button, event.button.x, event.button.y);
        break;
      }
      case SDL_MOUSEBUTTONUP: {
        if (event.button.button == 1) {
          mouse_left = false;
        }
        if (event.button.button == 3) {
          mouse_right = false;
        }
        mouse_button(false, event.button.button, event.button.x, event.button.y);
        break;
      }
      case SDL_JOYBUTTONDOWN: {
        joy_button(true, event.jbutton.button);
        break;
      }
      case SDL_JOYBUTTONUP: {
        joy_button(false, event.jbutton.button);
        break;
      }
      case SDL_JOYAXISMOTION: {
        if (event.jaxis.axis == 0) {
          joy_x = event.jaxis.value;
          std::cout << "Joy " << joy_x << std::endl;
        }
        if (event.jaxis.axis == 1) {
          joy_y = event.jaxis.value;
        }
        break;
      }
    }
  }
  return true;
}

void init_controller() {
  // Variables for controllers and joysticks

  // Enumerate joysticks
  std::cout << "Enumerating " << SDL_NumJoysticks() << " joysticks" << std::endl;
  for (int i = 0; i < SDL_NumJoysticks(); ++i) {

    // Check to see if joystick supports SDL's game controller interface
    if (SDL_IsGameController(i)) {
      controller = SDL_GameControllerOpen(i);
      if (controller) {
        std::cout << "Found a valid controller, named: " << SDL_GameControllerName(controller) << std::endl;
        break;  // Break after first available controller
      } else {
        std::cout << "Could not open game controller " << i << ": " << SDL_GetError() << std::endl;
      }

    // Controller interface not supported, try to open as joystick
    } else {
      std::cout << "Joystick " << i << " is not supported by the game controller interface" << std::endl;
/*
      joy = SDL_JoystickOpen(i);

      // Joystick is valid
      if (joy) {
        sprintf(S2D_msg,
          "Opened Joystick %i\n"
          "Name: %s\n"
          "Axes: %d\n"
          "Buttons: %d\n"
          "Balls: %d\n",
          i, SDL_JoystickName(joy), SDL_JoystickNumAxes(joy),
          SDL_JoystickNumButtons(joy), SDL_JoystickNumBalls(joy)
        );
        S2D_Log(S2D_msg, S2D_INFO);

      // Joystick not valid
      } else {
        sprintf(S2D_msg, "Could not open Joystick %i", i);
        S2D_Log(S2D_msg, S2D_ERROR);
      }

      break;  // Break after first available joystick
      */
    }
  }
}
  
SDL_Window* sdlWindow;

void createWindow(int w, int h, const char* name) {
  screen_w = w;
  screen_h = h;
  int sdl = SDL_Init(SDL_INIT_VIDEO | SDL_INIT_JOYSTICK);
  if (sdl != 0) {
    std::cout << "Error initializing SDL: " << SDL_GetError() << std::endl;
  }
  sdlWindow = SDL_CreateWindow(name,SDL_WINDOWPOS_UNDEFINED,SDL_WINDOWPOS_UNDEFINED, screen_w, screen_h, SDL_WINDOW_OPENGL);
  if (sdlWindow == NULL) {
    std::cout << "Error creating SDL window: " << SDL_GetError() << std::endl;
  }
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
  SDL_GLContext context = SDL_GL_CreateContext(sdlWindow);
  if (context == NULL) {
    std::cout << "Error creating context: " << SDL_GetError() << std::endl;
    std::cout << "Falling back to default context attributes" << std::endl;
    SDL_GL_ResetAttributes();
    context = SDL_GL_CreateContext(sdlWindow);
    if (context == NULL) {
      std::cout << "Error creating context: " << SDL_GetError() << std::endl;
    }
  }
  init_controller();
}

int getTick() {
    return SDL_GetTicks();
}

void delay(int ms) {
    SDL_Delay(ms);
}

void swapBuffers() {
  SDL_GL_SwapWindow(sdlWindow);
}

void startMainLoop() {
  while (true) {
    int beforeFrame = getTick();
    if (!processFrame()) {
      break;
    }
    swapBuffers();
    int afterFrame = getTick();
    int elapsed = afterFrame - beforeFrame;
    if (elapsed < 16) {
        delay(16 - elapsed);
    }
  }
}

#endif

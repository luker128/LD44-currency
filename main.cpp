#include <GLES3/gl3.h>
#include <iostream>
#include <sstream>

#include "main.h"
#include "lodepng.h"

int screen_w = 600;
int screen_h = 450;
int mouse_x = 0;
int mouse_y = 0;
bool mouse_left = false;
bool mouse_right = false;
int joy_x = 0;
int joy_y = 0;

unsigned int load_texture (const char *filename, unsigned int filter, int *out_w, int *out_h, float *out_u, float *out_v ) {
    std::vector<unsigned char> image;
    unsigned width, height;
    unsigned error = lodepng::decode(image, width, height, filename);

    // If there's an error, display it.
    if(error != 0)
    {
        std::cout << "error " << error << ": " << lodepng_error_text(error) << std::endl;
        return -1;
    }
    // Texture size must be power of two for the primitive OpenGL version this is written for. Find next power of two.
    size_t u2 = 1; while(u2 < width) u2 *= 2;
    size_t v2 = 1; while(v2 < height) v2 *= 2;
    // Ratio for power of two version compared to actual version, to render the non power of two image with proper size.
    double u3 = (double)width / u2;
    double v3 = (double)height / v2;

    // Make power of two version of the image.
    std::vector<unsigned char> image2(u2 * v2 * 4);
    for(size_t y = 0; y < height; y++)
        for(size_t x = 0; x < width; x++)
            for(size_t c = 0; c < 4; c++)
            {
                image2[4 * u2 * y + 4 * x + c] = image[4 * width * y + 4 * x + c];
            }

    GLuint txt_id;
    glGenTextures( 1, &txt_id );
    glBindTexture( GL_TEXTURE_2D, txt_id );
    glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA, u2, v2, 0, GL_RGBA, GL_UNSIGNED_BYTE, &image2[0] );
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, filter );
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, filter );

//    glGenTextures( 1, &txt_id );
//    glBindTexture( GL_TEXTURE_2D, txt_id );
//    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, filter );
//    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, filter );
//    glTexImage2D( GL_TEXTURE_2D, 0, 4, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image.data() );

    if (out_w) { *out_w = width; }
    if (out_h) { *out_h = height; }
    if (out_u) { *out_u = u3; }
    if (out_v) { *out_v = v3; }
    return txt_id;
    
//  return 0;
}

void destroy_texture(unsigned int texture) {
  glDeleteTextures(1, &texture);
}

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

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#include <emscripten/emscripten.h>
#include <emscripten/html5.h>
#include <emscripten/key_codes.h>


EM_BOOL key_callback(int eventType, const EmscriptenKeyboardEvent *e, void *userData) {
  //printf("key '%s', code '%s', charCode %lu, keyCode %lu\n", e->key, e->code, e->charCode, e->keyCode);
  bool pressed = true;
  if (eventType == EMSCRIPTEN_EVENT_KEYUP) {
    pressed = false;
  }
  key_press(pressed, e->keyCode, e->keyCode);

  return e->keyCode == DOM_VK_BACK_SPACE // Don't navigate away from this test page on backspace.
    || e->keyCode == DOM_VK_TAB // Don't change keyboard focus to different browser UI elements while testing.
    || (e->keyCode >= DOM_VK_F1 && e->keyCode <= DOM_VK_F24) // Don't F5 refresh the test page to reload.
    || e->ctrlKey // Don't trigger e.g. Ctrl-B to open bookmarks
    || e->altKey // Don't trigger any alt-X based shortcuts either (Alt-F4 is not overrideable though)
    || eventType == EMSCRIPTEN_EVENT_KEYPRESS || eventType == EMSCRIPTEN_EVENT_KEYUP; // Don't perform any default actions on these.
}


EM_BOOL mousedown_callback(int eventType, const EmscriptenMouseEvent *mouseEvent, void *userData) {
  mouse_button(true, mouseEvent->button, mouseEvent->canvasX, mouseEvent->canvasY);
  return true;
}

EM_BOOL mouseup_callback(int eventType, const EmscriptenMouseEvent *mouseEvent, void *userData) {
  mouse_button(false, mouseEvent->button, mouseEvent->canvasX, mouseEvent->canvasY);
  return true;
}

EM_BOOL mouse_callback(int eventType, const EmscriptenMouseEvent *mouseEvent, void *userData) {
  mouse_x = mouseEvent->canvasX;
  mouse_y = mouseEvent->canvasY;
  return true;
}



void createWindow(int w, int h, const char* name) {
  screen_w = w;
  screen_h = h;

  EmscriptenWebGLContextAttributes attr;
  emscripten_webgl_init_context_attributes(&attr);
  attr.majorVersion=2;
  attr.minorVersion=0;
  std::ostringstream os;
  os << "document.getElementById('canvas').width = " << w << ";\n";
  os << "document.getElementById('canvas').height = " << h << ";";
  emscripten_run_script(os.str().c_str());
  EMSCRIPTEN_WEBGL_CONTEXT_HANDLE ctx = emscripten_webgl_create_context("#canvas", &attr);
  emscripten_webgl_make_context_current(ctx);
  emscripten_set_keydown_callback(0, 0, 1, key_callback);
  emscripten_set_keyup_callback(0, 0, 1, key_callback);
  emscripten_set_keypress_callback(0, 0, 1, key_callback);
  emscripten_set_mousemove_callback(0, 0, 1, mouse_callback);
  emscripten_set_mousedown_callback(0, 0, 1, mousedown_callback);
  emscripten_set_mouseup_callback(0, 0, 1, mouseup_callback);
}

void void_processFrame() {
  processFrame();
}
void startMainLoop() {
  emscripten_set_main_loop(void_processFrame, 0, true);
}
bool processInput() {
  return true;
}

#else



#include <SDL2/SDL.h>

SDL_GameController *controller = NULL;

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

#ifdef __EMSCRIPTEN__
#include <GLES3/gl3.h>
#include <iostream>
#include <sstream>

#include "main.h"

#include <emscripten.h>
#include <emscripten/emscripten.h>
#include <emscripten/html5.h>
#include <emscripten/key_codes.h>

bool processFrame();

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

int mapMouseButton(int jsButton) {
  switch (jsButton) {
    case 0:
      return 1;
    case 1:
      return 2;
    case 2:
      return 3;
    default:
      return jsButton;
  }
}

EM_BOOL mousedown_callback(int eventType, const EmscriptenMouseEvent *mouseEvent, void *userData) {
  int button = mapMouseButton(mouseEvent->button);
  mouse_button(true, button, mouseEvent->canvasX, mouseEvent->canvasY);
  return true;
}

EM_BOOL mouseup_callback(int eventType, const EmscriptenMouseEvent *mouseEvent, void *userData) {
  int button = mapMouseButton(mouseEvent->button);
  mouse_button(false, button, mouseEvent->canvasX, mouseEvent->canvasY);
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

#endif

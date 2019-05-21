#pragma once

// Exports - implemnted in main.cpp
extern int screen_w;
extern int screen_h;
extern int mouse_x;
extern int mouse_y;
extern bool mouse_left;
extern bool mouse_right;
extern int joy_x;
extern int joy_y;
extern bool keys[];
void createWindow(int w, int h, const char* name);
int getTick();

// Imports - to bo impelemnnted by game
void gameInit();
bool gameLoop();
void gameCleanup();
void mouse_button(bool pressed, int button, int x, int y );
void mouse_wheel(int value);
void joy_button(bool pressed, int button);
void key_press(bool pressed, unsigned char, unsigned short key);


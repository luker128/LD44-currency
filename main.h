#pragma once

// Exports - implemnted in main.cpp
extern int screen_w;
extern int screen_h;
extern int mouse_x;
extern int mouse_y;
extern bool keys[];
void createWindow(int w, int h, const char* name);
int getTick();
unsigned int load_texture(const char* filename, unsigned int filter, int *out_w, int *out_h, float *out_u, float *out_v);
void destroy_texture(unsigned int texture);

// Imports - to bo impelemnnted by game
void gameInit();
bool gameLoop();
void gameCleanup();
void mouse_button( bool pressed, int button, int x, int y );
void key_press(bool pressed, unsigned char, unsigned short key);


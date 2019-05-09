#include <GLES3/gl3.h>
#include <iostream>

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



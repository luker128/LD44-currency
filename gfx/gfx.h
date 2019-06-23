#pragma once
#include <string>
#include <GLES3/gl3.h>

class string;

unsigned int load_texture(const char* filename, unsigned int filter, int *out_w, int *out_h, float *out_u, float *out_v);
void destroy_texture(unsigned int texture);

class Image {
  public:
    Image(const std::string &filename);
    GLuint getTexture() const { return texture; }
    void draw(int x, int y, int tw, int th) const;
    int getWidth() const { return w; };
    int getHeight() const { return h; };
  private:
    GLuint texture;
    int w;
    int h;
};


class Tilemap {
  public:
    Tilemap(const Image&, int width, int height, int tile_w, int tile_h, int tile_dx=0, int tile_dy=0, bool skipzero=false);
    void drawTilemap(int x, int y);
    void setAt(int x, int y, int newTile);
  private:
    void rebuild();
    const Image& image;
    const int width;
    const int height;
    const int tileWidth;
    const int tileHeight;
    const int tileDx;
    const int tileDy;
    const int sheetSize;
    const bool skipZero;
    int* grid;
    bool needRebuild;
    GLuint vao;
    GLuint buffer;
};

class SpriteSheet {
  public:
    SpriteSheet(const Image& image, int frame_w, int frame_h);
    SpriteSheet(const std::string&, int frame_w, int frame_h);
    ~SpriteSheet();
    void drawSprite(int x, int y, int frame);
    void drawSpriteFlipped(int x, int y, int frame);
    void drawSpriteScaled(int x, int y, int frame, float scale);
    void drawSpriteRotated(int x, int y, int frame, float rotation);
    int getFrameWidth() { return frameWidth; }
  private:
    const Image* ownImage;
    const Image& image;
    const int sheetSize;
    const int frameWidth;
    const int frameHeight;
};


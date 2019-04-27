#include <GLES3/gl3.h>
#include <iostream>
#include <string>
#include <cstring>
#include "gfx.h"
#include "main.h"

    const int TILE_SIZE = 32;
    const int MAP_W = 20;
    const int MAP_H = 15;
    const int POINTS_PER_TILE = 6;


Image::Image(const std::string &filename) {
  texture = load_texture(filename.c_str(), GL_NEAREST, &w, &h, nullptr, nullptr );
}

GLuint createShader(GLuint type, const std::string &source) {
  GLuint shader = glCreateShader(type);
  const char* text = source.c_str();
  glShaderSource(shader, 1, &text, NULL);
  glCompileShader(shader);
  GLint isCompiled;
  glGetShaderiv(shader, GL_COMPILE_STATUS, &isCompiled);
  if (isCompiled == GL_FALSE) {
    char buf[1024];
    glGetShaderInfoLog(shader, sizeof(buf), NULL, buf);
    std::cout << "Shader compilation failed" << std::endl << buf << std::endl;
  }
  return shader;
}

GLuint createProgram(const std::string &vertexShaderSource, const std::string &fragmentShaderSource) {
  GLuint vertexShader = createShader(GL_VERTEX_SHADER, vertexShaderSource);
  GLuint fragmentShader = createShader(GL_FRAGMENT_SHADER, fragmentShaderSource);
  GLuint program = glCreateProgram();
  glAttachShader(program, vertexShader);
  glAttachShader(program, fragmentShader);
  glLinkProgram(program);
  GLint isLinked;
  glGetProgramiv(program, GL_LINK_STATUS, &isLinked);
  if (isLinked == GL_FALSE) {
    char buf[1024];
    glGetProgramInfoLog(program, sizeof(buf), NULL, buf);
    std::cout << "Shader linking failed" << std::endl << buf << std::endl;
  }
  return program;
}

class TilemapShader {
  public:
    static TilemapShader& getInstance() {
      if (instance == nullptr) {
        instance = new TilemapShader();
      }
      return *instance;
    }
    GLuint buildVao(int map_w, int map_h, int tile_w, int tile_h, int sheetsize, int tile_dx, int tile_dy, GLuint *buffer);
    void drawTilemap(int scroll_x, int scroll_y, const Image& image, GLint vao, int w, int h, int sheetsize, bool skipzero);
    void updateTiles(GLint vao, GLint buffer, int* data, int map_w, int map_h);
  private:
    static TilemapShader* instance;
    TilemapShader();
    GLuint program;
    GLint a_positionLocation;
    GLint a_texcoordLocation;
    GLint a_tileLocation;
    GLuint positionBuffer;
    GLuint texcoordBuffer;
    GLuint tileBuffer;
    GLint u_scroll;
    GLint u_sheetsize;
    GLint u_skipzero;
    GLint u_screensize;
};


TilemapShader::TilemapShader() {
  std::string vertexShader = "#version 300 es \n"
    "in vec2 a_position; \n"
    "in vec2 a_texcoord; \n"
    "in float a_tile; \n"
    "out vec2 v_texcoord; \n"
    "uniform vec2 u_scroll; \n"
    "uniform float u_sheetsize; \n"
    "uniform bool u_skipzero; \n"
    "uniform vec2 u_screensize; \n"
    "void main() { \n"
    "  if (a_tile < 0.5 && u_skipzero) { \n"
    "    gl_Position = vec4(0.0, 0.0, 0.0, 1.0); \n"
    "  } else { \n"
    "    vec2 pos = a_position + u_scroll;\n"
    "    vec2 scaled_pos = ((pos/u_screensize) * 2.0 - 1.0) * vec2(1.0, -1.0); \n"
    "    gl_Position = vec4(scaled_pos, 1.0, 1.0); \n"
    "    float t = a_tile; \n"
    "    if (u_skipzero) { \n"
    "      t--; \n"
    "    } \n"
    "    v_texcoord = a_texcoord + vec2(mod(t, u_sheetsize)/u_sheetsize, floor(t/u_sheetsize)/u_sheetsize); \n"
    "  } \n "
    "} \n";
  std::string fragmentShader = "#version 300 es \n"
    "precision mediump float; \n"
    "in vec2 v_texcoord; \n"
    "uniform sampler2D u_texture; \n"
    "out vec4 outColor; \n"
    "void main() { \n"
    "  outColor = texture(u_texture, v_texcoord);\n"
    "}\n";

  program = createProgram(vertexShader, fragmentShader);
  glUseProgram(program);
  u_scroll = glGetUniformLocation(program, "u_scroll");
  u_sheetsize = glGetUniformLocation(program, "u_sheetsize");
  glUniform2f(u_scroll, 40, 40);
  u_skipzero = glGetUniformLocation(program, "u_skipzero");
  u_screensize = glGetUniformLocation(program, "u_screensize");
}

GLuint TilemapShader::buildVao(int map_w, int map_h, int tile_w, int tile_h, int sheetsize, int tile_dx, int tile_dy, GLuint* tileBuffer) {
  glUseProgram(program);
  GLuint vao;
  glGenVertexArrays(1, &vao);
  glBindVertexArray(vao);

  {
    const int pointsNumber = POINTS_PER_TILE * 2 * map_w * map_h;
    GLfloat positions[pointsNumber];
    GLfloat base_positions[] = {  0,   0,    0, (float)tile_h,   (float)tile_w,   0,
                                  0, (float)tile_h, (float)tile_w, 0,   (float)tile_w, (float)tile_h};
    for (int y=0; y<map_h; y++) {
      for (int x=0; x<map_w; x++) {
        int offset = x * POINTS_PER_TILE * 2 + y * map_w * POINTS_PER_TILE * 2;
        for (int i=0; i<POINTS_PER_TILE; i++) {
          int point_offset = offset + i * 2;
          positions[point_offset] = base_positions[i*2] + x * tile_dx;
          positions[point_offset+1] = base_positions[i*2+1] + y * tile_dy;
        }
      }
    }
    a_positionLocation = glGetAttribLocation(program, "a_position");
    glGenBuffers(1, &positionBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, positionBuffer);
    glBufferData(GL_ARRAY_BUFFER, pointsNumber * sizeof(positions[0]), positions, GL_STATIC_DRAW);
    glEnableVertexAttribArray(a_positionLocation);
    glVertexAttribPointer(a_positionLocation, 2, GL_FLOAT, false, 0, 0);
  }

  {
    const int pointsNumber = POINTS_PER_TILE * 2 * map_w * map_h;
     GLfloat positions[pointsNumber];
    GLfloat base_positions[] = {  0,   0,    0,   1/(float)sheetsize,   1/(float)sheetsize,   0,
                                  0,   1/(float)sheetsize,    1/(float)sheetsize,   0,   1/(float)sheetsize,   1/(float)sheetsize};
    for (int y=0; y<map_h; y++) {
      for (int x=0; x<map_w; x++) {
        int offset = x * POINTS_PER_TILE * 2 + y * map_w * POINTS_PER_TILE * 2;
        for (int i=0; i<POINTS_PER_TILE; i++) {
          int point_offset = offset + i * 2;
          positions[point_offset] = base_positions[i*2];
          positions[point_offset+1] = base_positions[i*2+1];
        }
      }
    }
    a_texcoordLocation = glGetAttribLocation(program, "a_texcoord");
    glGenBuffers(1, &texcoordBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, texcoordBuffer);
    glBufferData(GL_ARRAY_BUFFER, pointsNumber * sizeof(positions[0]), positions, GL_STATIC_DRAW);
    glEnableVertexAttribArray(a_texcoordLocation);
    glVertexAttribPointer(a_texcoordLocation, 2, GL_FLOAT, false, 0, 0);
  }

  {
    GLfloat base_positions[] = { 2, 3, 3, 3, 4, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                                 2, 3, 3, 3, 4, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                                 2, 3, 3, 3, 4, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                                 2, 3, 3, 3, 4, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                                 2, 3, 3, 3, 4, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                                 2, 3, 3, 3, 4, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                                 2, 3, 3, 3, 4, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                                 2, 3, 3, 3, 4, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                                 2, 3, 3, 3, 4, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                                 2, 3, 3, 3, 4, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                                 2, 3, 3, 3, 4, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                                 2, 3, 3, 3, 4, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                                 2, 3, 3, 3, 4, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                                 2, 3, 3, 3, 4, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                                 2, 3, 3, 3, 4, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
    GLfloat positions[map_w*map_h*POINTS_PER_TILE];
    for (int y=0; y<map_h; y++) {
      for (int x=0; x<map_w; x++) {
        int offset = x * POINTS_PER_TILE + y * map_w * POINTS_PER_TILE;
        for (int i=0; i<POINTS_PER_TILE; i++) {
          int point_offset = offset + i;
          positions[point_offset] = base_positions[x+y*map_w];
        }
      }
    }
    a_tileLocation = glGetAttribLocation(program, "a_tile");
    glGenBuffers(1, tileBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, *tileBuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(positions), positions, GL_DYNAMIC_DRAW);
    glEnableVertexAttribArray(a_tileLocation);
    glVertexAttribPointer(a_tileLocation, 1, GL_FLOAT, false, 0, 0);
  }
  return vao;
}

void TilemapShader::updateTiles(int vao, int vbo, int* data, int map_w, int map_h) {
    GLfloat positions[map_w*map_h*POINTS_PER_TILE];
    for (int y=0; y<map_h; y++) {
      for (int x=0; x<map_w; x++) {
        int offset = x * POINTS_PER_TILE + y * map_w * POINTS_PER_TILE;
        GLfloat tile = data[x+y*map_w];
        for (int i=0; i<POINTS_PER_TILE; i++) {
          int point_offset = offset + i;
          positions[point_offset] = tile;
        }
      }
    }
    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(positions), positions, GL_DYNAMIC_DRAW);
}

void TilemapShader::drawTilemap(int scroll_x, int scroll_y, const Image& image, GLint vao, int w, int h, int sheetsize, bool skipzero) {
  glUseProgram(program);
  glBindVertexArray(vao);
  glBindTexture(GL_TEXTURE_2D, image.getTexture());
  glUniform2f(u_scroll, scroll_x, scroll_y);
  glUniform1f(u_sheetsize, sheetsize);
  glUniform2f(u_screensize, screen_w, screen_h);
  glUniform1i(u_skipzero, skipzero);
  GLint offset = 0;
  GLsizei count = POINTS_PER_TILE * w * h;
  glDrawArrays(GL_TRIANGLES, offset, count);
}

Tilemap::Tilemap(const Image& image, int width, int height, int tile_w, int tile_h, int tile_dx, int tile_dy, bool skipzero)
  : image(image),
    width(width),
    height(height),
    tileWidth(tile_w),
    tileHeight(tile_h),
    tileDx(tile_dx ? tile_dx : tile_w),
    tileDy(tile_dy ? tile_dy : tile_h),
    sheetSize(image.getWidth()/tileWidth),
    skipZero(skipzero),
    grid(new int[width*height]),
    needRebuild(false) {
  memset(grid, 0, width*height*sizeof(int));
  TilemapShader& shader = TilemapShader::getInstance();
  vao = shader.buildVao(width, height, tileWidth, tileHeight, sheetSize, tileDx, tileDy, &buffer);
  rebuild();
}

void Tilemap::rebuild() {
  TilemapShader::getInstance().updateTiles(vao, buffer, grid, width, height);
  needRebuild = false;
}

void Tilemap::drawTilemap(int x, int y) {
  if (needRebuild) {
    rebuild();
  }
  TilemapShader::getInstance().drawTilemap(x, y, image, vao, width, height, sheetSize, skipZero);
}

void Tilemap::setAt(int x, int y, int newTile) {
  if (grid[x+y*width] != newTile) {
    grid[x+y*width] = newTile;
    needRebuild = true;
  }
}

TilemapShader* TilemapShader::instance = nullptr;



class SpriteShader {
  public:
    static SpriteShader& getInstance() {
      if (instance == nullptr) {
        instance = new SpriteShader();
      }
      return *instance;
    }
    void drawSprite(const Image& image, int x, int y, int tile, int sheetsize, int scale_x, int scale_y);
  private:
    static SpriteShader* instance;
    SpriteShader();
    GLuint vao;
    GLuint program;
    GLint a_positionLocation;
    GLint a_texcoordLocation;
    GLuint positionBuffer;
    GLuint texcoordBuffer;
    GLint u_scroll;
    GLint u_scale;
    GLint u_tile;
    GLint u_sheetsize;
    GLint u_skipzero;
    GLint u_screensize;
};

SpriteShader* SpriteShader::instance = nullptr;

SpriteShader::SpriteShader() {
  std::string vertexShader = "#version 300 es \n"
    "in vec2 a_position; \n"
    "in vec2 a_texcoord; \n"
    "out vec2 v_texcoord; \n"
    "uniform float u_tile; \n"
    "uniform vec2 u_scroll; \n"
    "uniform vec2 u_scale; \n"
    "uniform float u_sheetsize; \n"
    "uniform vec2 u_screensize; \n"
    "void main() { \n"
    "  v_texcoord = (a_texcoord/u_sheetsize) + vec2(mod(u_tile, u_sheetsize)/u_sheetsize, floor(u_tile/u_sheetsize)/u_sheetsize); \n"
    "  vec2 pos = (a_position * u_scale) + u_scroll;\n"
    "  vec2 scaled_pos = ((pos/u_screensize) * 2.0 - 1.0) * vec2(1.0, -1.0); \n"
    "  gl_Position = vec4(scaled_pos, 1.0, 1.0); \n"
    "}\n";
  std::string fragmentShader = "#version 300 es \n"
    "precision mediump float; \n"
    "in vec2 v_texcoord; \n"
    "uniform sampler2D u_texture; \n"
    "out vec4 outColor; \n"
    "void main() { \n"
    "  outColor = texture(u_texture, v_texcoord);\n"
    "}\n";

  program = createProgram(vertexShader, fragmentShader);
  glUseProgram(program);
  glGenVertexArrays(1, &vao);
  glBindVertexArray(vao);

  GLfloat positions[12] = {0,0, 0,1, 1,0,  0,1, 1,0, 1,1};
  a_positionLocation = glGetAttribLocation(program, "a_position");
  glGenBuffers(1, &positionBuffer);
  glBindBuffer(GL_ARRAY_BUFFER, positionBuffer);
  glBufferData(GL_ARRAY_BUFFER, sizeof(positions), positions, GL_STATIC_DRAW);
  glEnableVertexAttribArray(a_positionLocation);
  glVertexAttribPointer(a_positionLocation, 2, GL_FLOAT, false, 0, 0);

  GLfloat texcoords[12] = {0,0, 0,1, 1,0,  0,1, 1,0, 1,1};
  a_texcoordLocation = glGetAttribLocation(program, "a_texcoord");
  glGenBuffers(1, &texcoordBuffer);
  glBindBuffer(GL_ARRAY_BUFFER, texcoordBuffer);
  glBufferData(GL_ARRAY_BUFFER, sizeof(texcoords), texcoords, GL_STATIC_DRAW);
  glEnableVertexAttribArray(a_texcoordLocation);
  glVertexAttribPointer(a_texcoordLocation, 2, GL_FLOAT, false, 0, 0);

  u_scroll = glGetUniformLocation(program, "u_scroll");
  u_tile = glGetUniformLocation(program, "u_tile");
  u_scale = glGetUniformLocation(program, "u_scale");
  u_sheetsize = glGetUniformLocation(program, "u_sheetsize");
  u_screensize = glGetUniformLocation(program, "u_screensize");
}

void SpriteShader::drawSprite(const Image& image, int x, int y, int tile, int sheetsize, int scale_x, int scale_y) {
  glUseProgram(program);
  glBindVertexArray(vao);
  glBindTexture(GL_TEXTURE_2D, image.getTexture());
  glUniform1f(u_sheetsize, sheetsize);
  glUniform2f(u_scale, scale_x, scale_y);
  glUniform2f(u_screensize, screen_w, screen_h);
  glUniform2f(u_scroll, x, y);
  glUniform1f(u_tile, tile);
  glDrawArrays(GL_TRIANGLES, 0, POINTS_PER_TILE);
}



SpriteSheet::SpriteSheet(const Image& image, int frameWidth, int frameHeight)
  : image(image),
    sheetSize(image.getWidth()/frameWidth),
    frameWidth(frameWidth),
    frameHeight(frameHeight) {
  SpriteShader::getInstance();
}

void SpriteSheet::drawSprite(int x, int y, int frame) {
  SpriteShader::getInstance().drawSprite(image, x, y, frame, sheetSize, frameWidth, frameHeight);
}

void SpriteSheet::drawSpriteFlipped(int x, int y, int frame) {
  SpriteShader::getInstance().drawSprite(image, x, y, frame, sheetSize, -frameWidth, frameHeight);
}

void SpriteSheet::drawSpriteScaled(int x, int y, int frame, float scale) {
  SpriteShader::getInstance().drawSprite(image, x, y, frame, sheetSize, frameWidth*scale, frameHeight*scale);
}

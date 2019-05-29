#include <GLES3/gl3.h>
#include <iostream>
#include <string>
#include <cmath>
#include "primitive.h"
#include "main.h"



static GLuint createShader(GLuint type, const std::string &source) {
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

static GLuint createProgram(const std::string &vertexShaderSource, const std::string &fragmentShaderSource) {
  GLuint vertexShader = createShader(GL_VERTEX_SHADER, vertexShaderSource);
  GLuint fragmentShader = createShader(GL_FRAGMENT_SHADER, fragmentShaderSource);
  GLuint program = glCreateProgram();
  std::cout << "Compiling vertex shader" << std::endl;
  glAttachShader(program, vertexShader);
  std::cout << "Compiling fragment shader" << std::endl;
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



PrimitiveShader::PrimitiveShader() : red(1.0), green(1.0), blue(1.0), alpha(1.0), scrollX(0.0), scrollY(0.0) {
  std::string vertexShader = "#version 300 es \n"
    "in vec2 a_position; \n"
    "uniform vec2 u_scroll; \n"
    "uniform vec2 u_screensize; \n"
    "uniform vec2 u_texturePos; \n"
    "uniform float u_textureScale; \n"
    "out vec2 v_texcoord; \n"
    "void main() { \n"
    "  vec2 pos = a_position + u_scroll;\n"
    "  vec2 scaled_pos = ((pos/u_screensize) * 2.0 - 1.0) * vec2(1.0, -1.0); \n"
    "  gl_Position = vec4(scaled_pos, 1.0, 1.0); \n"
    "  v_texcoord = ((a_position+u_texturePos) / 128.0) * u_textureScale; \n"
    "} \n";
  std::string fragmentShader = "#version 300 es \n"
    "precision mediump float; \n"
    "out vec4 outColor; \n"
    "uniform vec4 u_color; \n"
    "in vec2 v_texcoord; \n"
    "uniform sampler2D u_texture; \n"
    "uniform bool u_useTexture; \n"
    "void main() { \n"
    "  if (u_useTexture) { \n"
    "    outColor = texture(u_texture, v_texcoord);\n"
    "  } \n"
    "  else { \n"
    "    outColor = u_color; \n"
    "  } \n"
    "}\n";

  program = createProgram(vertexShader, fragmentShader);
  glUseProgram(program);
  glGenVertexArrays(1, &vao);
  glBindVertexArray(vao);

  GLfloat positions[4] = {0,0, 1,1};
  GLuint a_positionLocation = glGetAttribLocation(program, "a_position");
  glGenBuffers(1, &positionBuffer);
  glBindBuffer(GL_ARRAY_BUFFER, positionBuffer);
  glBufferData(GL_ARRAY_BUFFER, sizeof(positions), positions, GL_DYNAMIC_DRAW);
  glEnableVertexAttribArray(a_positionLocation);
  glVertexAttribPointer(a_positionLocation, 2, GL_FLOAT, false, 0, 0);

  u_scroll = glGetUniformLocation(program, "u_scroll");
  u_screensize = glGetUniformLocation(program, "u_screensize");
  u_color = glGetUniformLocation(program, "u_color");
  u_useTexture = glGetUniformLocation(program, "u_useTexture");
  u_textureScale = glGetUniformLocation(program, "u_textureScale");
  u_texturePos = glGetUniformLocation(program, "u_texturePos");
}


void PrimitiveShader::setColor(float r, float g, float b, float a) {
  red = r;
  green = g;
  blue = b;
  alpha = a;
}

void PrimitiveShader::setScroll(int x, int y) {
  scrollX = x;
  scrollY = y;
}

void PrimitiveShader::setScale(float s) {
  scale = s;
}


void PrimitiveShader::drawLine(float x0, float y0, float x1, float y1) {
  GLfloat positions[4] = {x0,y0, x1,y1};
  glBindBuffer(GL_ARRAY_BUFFER, positionBuffer);
  glBufferData(GL_ARRAY_BUFFER, sizeof(positions), positions, GL_DYNAMIC_DRAW);
  executeDraw(GL_LINES, 2);
}

void PrimitiveShader::drawRectangle(float x0, float y0, float x1, float y1) {
  GLfloat positions[12] = {x0,y0, x1,y0, x0,y1,  x1,y0, x0,y1, x1,y1};
  glBindBuffer(GL_ARRAY_BUFFER, positionBuffer);
  glBufferData(GL_ARRAY_BUFFER, sizeof(positions), positions, GL_DYNAMIC_DRAW);
  executeDraw(GL_TRIANGLES, 6);
}

void PrimitiveShader::drawConvexPolygon(const std::vector<float>& points, float texture_scale, float texture_pos_x, float texture_pos_y) {
  const GLfloat* positions = points.data();
  glBindBuffer(GL_ARRAY_BUFFER, positionBuffer);
  glBufferData(GL_ARRAY_BUFFER, points.size() * sizeof(float), positions, GL_DYNAMIC_DRAW);
  glUseProgram(program);
  glBindVertexArray(vao);
  glUniform2f(u_screensize, screen_w*scale, screen_h*scale);
  glUniform4f(u_color, red, green, blue, alpha);
  glUniform2f(u_scroll, scrollX, scrollY);
  glUniform1f(u_textureScale, texture_scale);
  glUniform2f(u_texturePos, texture_pos_x, texture_pos_y);
  glUniform1i(u_useTexture, 1);
  glDrawArrays(GL_TRIANGLES, 0, points.size()/2);
}

void PrimitiveShader::drawCircleOutline(float x0, float y0, float r) {
  const int detail = 16;
  GLfloat positions[detail*2];
  for (int i=0; i<detail; i++) {
    float f = (i/(float)detail) * 2 * M_PI;
    float x = x0 + sin(f) * r;
    float y = y0 + cos(f) * r;
    positions[i*2+0] = x;
    positions[i*2+1] = y;
  }
  glBindBuffer(GL_ARRAY_BUFFER, positionBuffer);
  glBufferData(GL_ARRAY_BUFFER, sizeof(positions), positions, GL_DYNAMIC_DRAW);
  executeDraw(GL_LINE_LOOP, detail);
}

void PrimitiveShader::drawCircle(float x0, float y0, float r) {
  const int detail = 16;
  GLfloat positions[(detail+1)*2];
  for (int i=0; i<detail; i++) {
    float f = (i/(float)detail) * 2 * M_PI;
    float x = x0 + sin(f) * r;
    float y = y0 + cos(f) * r;
    positions[i*2+0] = x;
    positions[i*2+1] = y;
  }
  positions[detail*2+0] = x0;
  positions[detail*2+1] = y0;
  glBindBuffer(GL_ARRAY_BUFFER, positionBuffer);
  glBufferData(GL_ARRAY_BUFFER, sizeof(positions), positions, GL_DYNAMIC_DRAW);
  executeDraw(GL_TRIANGLE_FAN, detail+1);
}

void PrimitiveShader::executeDraw(int primitive, int numVertex, bool useTexture) {
  glUseProgram(program);
  glBindVertexArray(vao);
  glUniform2f(u_scroll, scrollX, scrollY);
  glUniform2f(u_screensize, screen_w*scale, screen_h*scale);
  glUniform4f(u_color, red, green, blue, alpha);
  glUniform1i(u_useTexture, useTexture);
  glDrawArrays(primitive, 0, numVertex);
}


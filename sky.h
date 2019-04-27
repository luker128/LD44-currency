#include <GLES3/gl3.h>
#include <string>
#include "main.h"

#include <GLES3/gl3.h>

GLuint createProgram(const std::string &vertexShaderSource, const std::string &fragmentShaderSource);

class Sky {
  public:
    Sky(
    float col1_r=1.0, float col1_g=0.0, float col1_b=0.0, float col1_a=1.0,
    float col2_r=0.0, float col2_g=0.0, float col2_b=1.0, float col2_a=1.0,
    float col3_r=0.0, float col3_g=0.0, float col3_b=1.0, float col3_a=1.0,
    float col4_r=0.0, float col4_g=1.0, float col4_b=1.0, float col4_a=1.0)
      : 
    col1_r(col1_r), col1_g(col1_g), col1_b(col1_b), col1_a(col1_a),
    col2_r(col2_r), col2_g(col2_g), col2_b(col2_b), col2_a(col2_a),
    col3_r(col3_r), col3_g(col3_g), col3_b(col3_b), col3_a(col3_a),
    col4_r(col4_r), col4_g(col4_g), col4_b(col4_b), col4_a(col4_a)
    {
      std::string vertexShader = "#version 300 es \n"
        "in vec2 a_position; \n"
        "out vec2 v_texcoord; \n"
        "void main() { \n"
        "  gl_Position = vec4(a_position, 1.0, 1.0); \n"
        "  v_texcoord = vec2(a_position); \n"
        "} \n";
      std::string fragmentShader = "#version 300 es \n"
        "precision mediump float; \n"
        "in vec2 v_texcoord; \n"
        "out vec4 outColor; \n"
        "uniform vec2 u_scroll; \n"
        "uniform vec4 red; \n"
        "uniform vec4 blue; \n"
        "uniform vec4 yellow; \n"
        "uniform vec4 cyan; \n"
        "vec4 tween(vec4 a, vec4 b, float step) { \n"
        "  return step * a + (1.0-step) * b; \n"
        "} \n"
        "void main() { \n"
        "  float y = (v_texcoord.y+1.0)/2.0; \n"
        "  float a = y; \n"
        "  float one_minus_a = 1.0-a; \n"
        /*
        "  vec4 red =    vec4(1.0, 0.0, 0.0, 1.0); \n"
        "  vec4 blue =   vec4(0.0, 0.0, 1.0, 1.0); \n"
        "  vec4 yellow = vec4(1.0, 1.0, 0.0, 1.0); \n"
        "  vec4 cyan =   vec4(0.0, 1.0, 1.0, 1.0); \n"
        */
        "  vec4 gradient_1 = tween(blue, red, a); \n"
        "  vec4 gradient_2 = tween(cyan, yellow, a); \n"
        "  outColor = tween(gradient_1, gradient_2, u_scroll.x); \n"
        "}\n";

      program = createProgram(vertexShader, fragmentShader);
      glUseProgram(program);
      glGenVertexArrays(1, &vao);
      glBindVertexArray(vao);

      GLfloat positions[12] = {-1,-1, -1,+1, +1,-1,  -1,+1, +1,-1, +1,+1};
      GLuint a_positionLocation = glGetAttribLocation(program, "a_position");
      GLuint positionBuffer;
      glGenBuffers(1, &positionBuffer);
      glBindBuffer(GL_ARRAY_BUFFER, positionBuffer);
      glBufferData(GL_ARRAY_BUFFER, sizeof(positions), positions, GL_STATIC_DRAW);
      glEnableVertexAttribArray(a_positionLocation);
      glVertexAttribPointer(a_positionLocation, 2, GL_FLOAT, false, 0, 0);
      u_scroll = glGetUniformLocation(program, "u_scroll");
      u_red = glGetUniformLocation(program, "red");
      u_blue = glGetUniformLocation(program, "blue");
      u_yellow = glGetUniformLocation(program, "yellow");
      u_cyan = glGetUniformLocation(program, "cyan");
    }
    void draw(float scroll) {
      glUseProgram(program);
      glBindVertexArray(vao);
      glUniform2f(u_scroll, scroll, scroll);
      glUniform4f(u_red,   col1_r, col1_g, col1_b, col1_a);
      glUniform4f(u_blue,  col2_r, col2_g, col2_b, col2_a);
      glUniform4f(u_cyan,  col3_r, col3_g, col3_b, col3_a);
      glUniform4f(u_yellow,col4_r, col4_g, col4_b, col4_a);
      glDrawArrays(GL_TRIANGLES, 0, 6);
    }
  private:
    float col1_r, col1_g, col1_b, col1_a;
    float col2_r, col2_g, col2_b, col2_a;
    float col3_r, col3_g, col3_b, col3_a;
    float col4_r, col4_g, col4_b, col4_a;
    GLuint program;
    GLuint u_scroll;
    GLuint u_red;
    GLuint u_blue;
    GLuint u_yellow;
    GLuint u_cyan;
    GLuint vao;
};


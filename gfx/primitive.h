#pragma once
#include <GLES3/gl3.h>
#include <vector>

class PrimitiveShader {
  public:
    PrimitiveShader();
    void setColor(float r, float g, float b, float a);
    void setScroll(int x, int y);
    void setScale(float scale);
    void drawLine(float x0, float y0, float x1, float y1);
    void drawRectangle(float x0, float y0, float x1, float y1);
    void drawConvexPolygon(const std::vector<float>& points, float textureScale, float texture_x, float texture_y);
    void drawCircle(float x, float y, float r);
    void drawCircleOutline(float x, float y, float r);
  private:
    void executeDraw(int primitive, int numVertex, bool useTexture = false);
    GLuint program;
    GLuint vao;
    GLuint positionBuffer;
    GLuint u_useTexture;
    GLuint u_scroll;
    GLuint u_screensize;
    GLuint u_textureScale;
    GLuint u_texturePos;
    GLuint u_color;
    float red;
    float green;
    float blue;
    float alpha;
    int scrollX;
    int scrollY;
    float scale = 1.0;
};


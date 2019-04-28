#include <float.h>
#include <vector>
#include <GLES3/gl3.h>
#include <cmath>
#include <cstdlib>
#include <string>
#include <tuple>
#include <vector>
#include <sstream>
#include <iostream>
#include "main.h"
#include "primitive.h"
#include "gfx.h"


struct Point {
  float x;
  float y;
  Point(float x, float y) : x(x), y(y) {}
};

struct Coin {
  Point position;
  Point velocity;
  float rotation = 0.0;
  bool sliding = false;
  Coin(int x, int y) : position(x, y), velocity(0, 0) {}
};

const int COIN_SIZE = 32;
const int POINT_SIZE = 3;
const float GRAVITY = 0.1;
const float GRAVITY_MAX = 3;

struct Polygon {
  std::vector<Point*> points;
  int textureId;
};
//typedef std::vector<Point*> Polygon;

float scalarMult(const Point& a, const Point& b) {
  return a.x * b.x + a.y * b.y;
}

Point operator*(const Point& v, float s) {
  return Point(v.x * s, v.y * s);
}

Point operator/(const Point& v, float s) {
  return Point(v.x / s, v.y / s);
}

float len(const Point& v) {
  return sqrt(v.x*v.x + v.y*v.y);
}

float len2(const Point& v) {
  return v.x*v.x + v.y*v.y;
}

Point operator-(const Point& a, const Point& b) {
  return Point(a.x - b.x, a.y - b.y);
}

std::ostream& operator<<(std::ostream& os, const Point& v) {
  os << "(" << v.x << ", " << v.y << ")";
  return os;
}

float sign(float a) {
  if (a > 0.0) {
    return +1.0;
  }
  if (a < 0.0) {
    return -1.0;
  }
  return 0.0;
}

class Editor {

    Editor(const Editor&) = delete;

  public:

    Editor() :
      coinImage("data/coin.png", 32, 32)
      // https://www.deviantart.com/strapaca/art/Diamond-metal-floor-seamless-texture-787512023
//      terrainImage("data/steel.png") {
      //  https://www.deviantart.com/strapaca/art/Fresh-dark-green-gras-seamless-texture-782082733
//      terrainImage("data/grass.png") {
      // https://www.deviantart.com/strapaca/art/Brick-wall-seamless-texture-782082949
//      terrainImage("data/bricks.png")
    {
      textures.emplace_back("data/bricks.png");
      textures.emplace_back("data/steel.png");
      textures.emplace_back("data/grass.png");
    }

    void keyPressEvent(bool pressed, unsigned char key, unsigned short code) {
      if (pressed == true) {
        keyPress(key);
      }
    }

    void mouseButton(bool pressed, int button) {
      if (pressed == false && button == 1) {
        mouseUpLeft();
      }
      if (pressed == false && button == 3) {
        mouseUpRight();
      }
      if (pressed == false && button == 2) {
        mouseUpMiddle();
      }
      if (pressed == true && button == 1) {
        mouseDownLeft();
      }
      if (pressed == true && button == 3) {
        mouseDownRight();
      }
      if (pressed == true && button == 2) {
        mouseDownMiddle();
      }
    }

    bool gameLoop() {
      update();
      draw();
      return true;
    }

  private:

    void mouseDownLeft() {}
    void mouseUpLeft() {
      if (newPolygon != nullptr) {
        addPointToPolygon(mouse_x, mouse_y, *newPolygon);
      }
      else {
        // move existing point
        if (currentPoint == nullptr) {
          auto p = getPointAt(mouse_x, mouse_y);
          std::cout << p << std::endl;
          currentPoint = p;
        }
        else {
          currentPoint = nullptr;
        }
      }
    }

    void mouseDownRight() {}
    void mouseUpRight() {
    }

    void mouseDownMiddle() {}
    void mouseUpMiddle() {
//      std::cout << getPolygonAt(mouse_x, mouse_y) << std::endl;
//      std::cout << findNearestLine(mouse_x, mouse_y) << std::endl;
    }

    void keyPress(int key) {
      std::cout << key << std::endl;
      if (key == 32) { // SPACE
        if (newPolygon == nullptr)  {
          newPolygon = new Polygon();
          addPointToPolygon(mouse_x, mouse_y, *newPolygon);
        }
        else {
          Point* firstPoint = newPolygon->points.front();
          newPolygon->points.push_back(firstPoint);
          polygons.push_back(newPolygon);
          newPolygon = nullptr;
        }
      }
      if (key == 'f') {
        drawFilledPolygons = !drawFilledPolygons;
      }
      if (key == '\r') {
        spawnCoin(mouse_x, mouse_y);
      }
      if (key == 75) { // pg up
        Polygon* poly = getPolygonAt(mouse_x, mouse_y);
        poly->textureId++;
        if (poly->textureId == textures.size()) {
          poly->textureId = 0;
        }
      }
      if (key == 78) { // pg dn
        Polygon* poly = getPolygonAt(mouse_x, mouse_y);
        poly->textureId--;
        if (poly->textureId == -1) {
          poly->textureId = textures.size()-1;
        }
      }

    }

    void addPointToPolygon(int x, int y, Polygon& polygon) {
      Point* oldPoint = getPointAt(x, y);
      if (oldPoint == nullptr) {
        Point* newPoint = new Point(x, y);
        polygon.points.push_back(newPoint);
      }
      else {
        polygon.points.push_back(oldPoint);
      }
    }

    Point* getPointAt(int x, int y) {
      for (const auto& poly: polygons) {
        for (auto &point: poly->points) {
          int dx = abs(x - point->x);
          int dy = abs(y - point->y);
          if (std::max(dx, dy) <= POINT_SIZE) {
            return point;
          }
        }
      }
      return nullptr;
    }

    bool sideOfLine(float x, float y, Point* a, Point* b) {
      // std::cout << "checking side " << x << ", " << y << " of line "
      //  << a->x << ", " << a->y << " - " << b->x << ", " << b->y;

      float dy = (b->y - a->y);
      float dx = (b->x - a->x);
      float direction = dy/dx;
      float position = a->y - direction*a->x;

      bool relation = y > direction*x + position;
      // std::cout << std::endl << "direction = " << dx << std::endl;
      // std::cout << "relation = " << relation << std::endl;
      bool result = (relation == (dx < 0));
      if (result == true) {
        // std::cout << " FALSE" << std::endl;
        return false;
      }
      else {
        // std::cout << " TRUE" << std::endl;
        return true;
      }
    }

    bool isInPolygon(float x, float y, const Polygon& poly) {
      // std::cout << "checking if " << x << ", " << y << " is in polygon" << std::endl;
      for (size_t i=0; i+1<poly.points.size(); i++) {
        bool side = sideOfLine(x, y, poly.points[i], poly.points[i+1]);
        if (side == false) {
          return false;
        }
      }
      return true;
    }

    Polygon* getPolygonAt(float x, float y) {
      for (Polygon* polygon: polygons) {
        if (isInPolygon(x, y, *polygon)) {
          return polygon;
        }
      }
      return nullptr;
    }

    float distanceToLine(float x, float y, Point* a, Point* b) {
      float dy = (b->y - a->y);
      float dx = (b->x - a->x);

      float direction = dy/dx;
      float position = a->y - direction*a->x;
      float A = -direction;
      float B = 1; // or 0 if dx == 0?
      float C = -position;
      float dist = fabs(A*x + B*y + C) / sqrt(A*A+B*B);

      // czy punkty styku z prostą leży na odcinku?
      float lenC = sqrt(dx*dx + dy*dy);
      float lenA = sqrt( (a->x-x)*(a->x-x) + (a->y-y)*(a->y-y) );
      float lenB = sqrt( (b->x-x)*(b->x-x) + (b->y-y)*(b->y-y) );
      if (lenA > lenC || lenB > lenC) {
        if (lenA*lenA > lenB*lenB + lenC*lenC ||
            lenB*lenB > lenA*lenA + lenC*lenC) {
          if (lenA < lenB) {
            return lenA;
          }
          else {
            return lenB;
          }
        }
      }
      return dist;
    }

    struct LineInPoly {
      Polygon* polygon;
      size_t pointIndex;
    };

    std::tuple<float, LineInPoly> findNearestLine(float x, float y) {
      // std::cout << "finding line nearest to " << x << ", " << y << std::endl;
      float min = FLT_MAX;
      LineInPoly nearestLine = {nullptr, 0};
      for (Polygon* polygon: polygons) {
        for (size_t i=0; i+1<polygon->points.size(); i++) {
          float distance = distanceToLine(x, y, polygon->points[i], polygon->points[i+1]);
          if (distance < min) {
            min = distance;
            nearestLine = {polygon, i};
          }
        }
      }
      return std::make_tuple(min, nearestLine);
    }


    void applyVelocity(Coin& coin, bool recursion = false) {
      float newX = coin.position.x + coin.velocity.x;
      float newY = coin.position.y + coin.velocity.y;
      /*
      Polygon* collidingPolygon = getPolygonAt(newX, newY);
      if (collidingPolygon == nullptr) {
      */
      float lineDistance;
      LineInPoly nearestLine;
      std::tie(lineDistance, nearestLine)  = findNearestLine(newX, newY);
      if (lineDistance > COIN_SIZE) {
        coin.position.x = newX;
        coin.position.y = newY;
        if (!recursion) {
          coin.sliding = false;
        }
        mCurrentLine.polygon = nullptr;
      }
      else {
        if (recursion) {
          coin.velocity = {0.0, 0.0};
          return;
        }
        mCurrentLine = nearestLine;
        float lineDx = (nearestLine.polygon->points[nearestLine.pointIndex+1]->x - nearestLine.polygon->points[nearestLine.pointIndex]->x);
        float lineDy = (nearestLine.polygon->points[nearestLine.pointIndex+1]->y - nearestLine.polygon->points[nearestLine.pointIndex]->y);
        float lineLen = sqrt(lineDx*lineDx + lineDy*lineDy);
        float scal = scalarMult(coin.velocity, Point(lineDx, lineDy));
        float magn = scal / (lineDx*lineDx + lineDy*lineDy);
        coin.velocity = Point(lineDx, lineDy) * magn;

        coin.sliding = true;
        applyVelocity(coin, true); // after adjustment, try again
      }
    }

    void updateCoin(Coin& coin) {
      if (std::fabs(coin.velocity.y) < GRAVITY_MAX) {
        coin.velocity.y += GRAVITY;
      }
      applyVelocity(coin);
    }

    void update() {
      if (currentPoint != nullptr) {
        currentPoint->x = mouse_x;
        currentPoint->y = mouse_y;
      }
      if (coin != nullptr) {
        updateCoin(*coin);
      }
    }

    void drawPoint(const Point& point) {
      static const int SIZE = POINT_SIZE;
      prim.drawRectangle(point.x-SIZE, point.y-SIZE,
                         point.x+SIZE, point.y+SIZE);
    }

    void drawPolygon(const Polygon& polygon) {
      if (drawFilledPolygons) {
        std::vector<float> positions;
        for (Point* point: polygon.points) {
          positions.push_back(point->x);
          positions.push_back(point->y);
        }
        glBindTexture(GL_TEXTURE_2D, textures[polygon.textureId].getTexture());
        prim.drawConvexPolygon(positions);
      }
      else {
        for (const auto& point: polygon.points) {
          drawPoint(*point);
        }
        for (size_t i=0; (i+1)<polygon.points.size(); i++) {
          if (mCurrentLine.polygon == &polygon && mCurrentLine.pointIndex == i) {
            prim.setColor(1,0,0,1);
          }
          else {
            prim.setColor(1,1,1,1);
          }
          prim.drawLine(polygon.points[i]->x, polygon.points[i]->y,
                        polygon.points[i+1]->x, polygon.points[i+1]->y);
        }
      }
    }

    void draw() {
      glClear(GL_COLOR_BUFFER_BIT);
      prim.setColor(1,1,1,1);
      for (Polygon* poly: polygons) {
        drawPolygon(*poly);
      }
      prim.setColor(1,1,0,1);
      if (newPolygon != nullptr) {
        drawPolygon(*newPolygon);
      }
      if (coin != nullptr) {
        if (coin->velocity.x != 0) {
          coin->rotation += coin->velocity.x / COIN_SIZE;
        }
        coinImage.drawSpriteRotated(coin->position.x, coin->position.y, 0, coin->rotation);
        prim.drawLine(coin->position.x, coin->position.y,
                      coin->position.x + coin->velocity.x * 30,
                      coin->position.y + coin->velocity.y * 30
            );
      }

//      float r = findNearestLine(mouse_x, mouse_y);
//      prim.drawCircleOutline(mouse_x, mouse_y, r);
    }

    void spawnCoin(int x, int y) {
      std::cout << "Spawning coin at " << x << ", " << y << std::endl;
      delete coin;
      coin = new Coin(x,y);
    }

    Coin* coin = nullptr;
    bool drawFilledPolygons = false;
    PrimitiveShader prim;
    SpriteSheet coinImage;
//    Image terrainImage;
    Polygon* newPolygon = nullptr;
    std::vector<Polygon*> polygons;
    Point* currentPoint = nullptr;
    LineInPoly mCurrentLine;
    std::vector<Image> textures;
};






Editor* editor = nullptr;


void key_press(bool pressed, unsigned char key, unsigned short code) {
  editor->keyPressEvent(pressed, key, code);
}

void mouse_button(bool pressed, int button, int x, int y ) {
  editor->mouseButton(pressed, button);
}

bool gameLoop() {
  return editor->gameLoop();
}

void gameInit() {
  createWindow(960, 540, "Currency - Editor");
  glViewport(0, 0, screen_w, screen_h);
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glClearColor(0.4, 0.4, 0.4, 1.0);
  editor = new Editor();
}

void gameCleanup() {
  delete editor;
}


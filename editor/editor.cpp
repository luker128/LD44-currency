#include <float.h>
#include <vector>
#include <GLES3/gl3.h>
#include <cmath>
#include <cstdlib>
#include <string>
#include <map>
#include <tuple>
#include <vector>
#include <sstream>
#include <iostream>
#include <fstream>
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
      coinImage("data/coin5.png", 64, 64)
    {
      // https://www.deviantart.com/strapaca/art/Brick-wall-seamless-texture-782082949
      textures.emplace_back("data/bricks.png");
      // https://www.deviantart.com/strapaca/art/Diamond-metal-floor-seamless-texture-787512023
      textures.emplace_back("data/steel.png");
      //  https://www.deviantart.com/strapaca/art/Fresh-dark-green-gras-seamless-texture-782082733
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
      mX = mouse_x - scrollX;
      mY = mouse_y - scrollY;
      update();
      draw();
      return true;
    }

  private:

    void mouseDownLeft() {}
    void mouseUpLeft() {
      if (newPolygon != nullptr) {
        addPointToPolygon(mX, mY, *newPolygon);
      }
      else {
        // move existing point
        if (currentPoint == nullptr) {
          auto p = getPointAt(mX, mY);
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

    void mouseDownMiddle() {
      scrolling = true;
      scrollStartX = mouse_x;
      scrollStartY = mouse_y;
    }
    void mouseUpMiddle() {
      scrolling = false;
//      std::cout << getPolygonAt(mX, mY) << std::endl;
//      std::cout << findNearestLine(mX, mY) << std::endl;
    }

    void keyPress(int key) {
      std::cout << key << std::endl;
      if (key == 127) { // delete
        delete coin;
        coin = nullptr;
      }
      if (key == 32) { // SPACE
        if (newPolygon == nullptr)  {
          newPolygon = new Polygon();
          addPointToPolygon(mX, mY, *newPolygon);
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
        spawnCoin(mX, mY);
      }
      if (key == 75) { // pg up
        Polygon* poly = getPolygonAt(mX, mY);
        poly->textureId++;
        if (poly->textureId == textures.size()) {
          poly->textureId = 0;
        }
      }
      if (key == 78) { // pg dn
        Polygon* poly = getPolygonAt(mX, mY);
        poly->textureId--;
        if (poly->textureId == -1) {
          poly->textureId = textures.size()-1;
        }
      }
      if (key == 'l') {
        loadLevel();
      }
      if (key == 's') {
        saveLevel();
      }
      if (key == 80) {
        coin->velocity.x -= 0.1;
      }
      if (key == 79) {
        coin->velocity.x += 0.1;
      }
    }

    void saveLevel() {
      std::map<Point*, int> pointIds;
      std::vector<Point*> pointsById;
      int count = 0;
      for (Polygon* polygon: polygons) {
        for (Point* point: polygon->points) {
          if (pointIds.find(point) == pointIds.end()) {
            pointIds[point] = count;
            count++;
            pointsById.push_back(point);
          }
        }
      }
      std::ofstream file("level.txt");
      file << pointIds.size() << std::endl;
      for (const auto& p: pointsById) {
        file << p->x << " " << p->y << std::endl;
      }
      file << polygons.size() << std::endl;
      for (Polygon* polygon: polygons) {
        file << polygon->textureId << " ";
        file << polygon->points.size();
        for (Point* point: polygon->points) {
          file << " " << pointIds[point];
        }
        file << std::endl;
      }
    }

    void loadLevel() {
      std::ifstream file("level.txt");
      int numPoints;
      file >> numPoints;
      std::vector<Point*> points;
      for (int i=0; i<numPoints; i++) {
        int x;
        int y;
        file >> x >> y;
        points.push_back(new Point(x, y));
      }
      int numPolygons;
      file >> numPolygons;
      for (int i=0; i<numPolygons; i++) {
        Polygon* polygon = new Polygon;
        int textureId;
        int numPoints;
        file >> textureId >> numPoints;
        polygon->textureId = textureId;
        for (int j=0; j<numPoints; j++) {
          int pointId;
          file >> pointId;
          std::cout << pointId << " ";
          polygon->points.push_back(points[pointId]);
        }
        std::cout << std::endl;
        polygons.push_back(polygon);
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


    void applyVelocity(Coin& coin, int recursion = 0) {
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
        if (recursion > 5) {
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
        applyVelocity(coin, recursion+1); // after adjustment, try again
      }
    }

    float getJoyValue() {
      // std::cout << "input:   " << joy_x;
      int reduced = joy_x / 1024;
      // std::cout << "reduced: " << reduced << std::endl;
      float scaled = reduced / 30.0;
      // std::cout << "scaled:  " << scaled << std::endl;
      if (scaled > 1.0) {
        scaled = 1.0;
      }
      if (scaled < -1.0) {
        scaled = -1.0;
      }
      // std::cout << "clamped: " << scaled << std::endl;
      return scaled;
    }

    void updateCoin(Coin& coin) {
      if (std::fabs(coin.velocity.y) < GRAVITY_MAX) {
        coin.velocity.y += GRAVITY;
      }
      float joyValue = getJoyValue();
      coin.velocity.x += joyValue * 0.03;
      applyVelocity(coin);
    }

    void update() {
      if (scrolling) {
        scrollX += (mouse_x - scrollStartX);
        scrollY += (mouse_y - scrollStartY);
        scrollStartX = mouse_x;
        scrollStartY = mouse_y;
      }
      if (currentPoint != nullptr) {
        currentPoint->x = mX;
        currentPoint->y = mY;
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
      if (coin != nullptr) {
        prim.setScroll((screen_w/2)-coin->position.x,
                       (screen_h/2)-coin->position.y);
      }
      else {
        prim.setScroll(scrollX, scrollY);
      }
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
        //coinImage.drawSpriteRotated(coin->position.x+scrollX, coin->position.y+scrollY, 0, coin->rotation);
        coinImage.drawSpriteRotated(screen_w / 2, screen_h /2, 0, coin->rotation);
        prim.drawLine(coin->position.x, coin->position.y,
                      coin->position.x + coin->velocity.x * 30,
                      coin->position.y + coin->velocity.y * 30
            );
      }

//      float r = findNearestLine(mX, mY);
//      prim.drawCircleOutline(mX, mY, r);
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
    float scrollX = 0;
    float scrollY = 0;
    bool scrolling = false;
    int scrollStartX;
    int scrollStartY;
    int mX;
    int mY;
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
  glClearColor(101/255.0, 184/255.0, 227/225.0, 1.0);
  editor = new Editor();
}

void gameCleanup() {
  delete editor;
}


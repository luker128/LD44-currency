#include <float.h>
#include <GLES3/gl3.h>
#include <cmath>
#include <cstdlib>
#include <string>
#include <set>
#include <map>
#include <tuple>
#include <vector>
#include <sstream>
#include <iostream>
#include <fstream>
#include "sys/main.h"
#include "gfx/primitive.h"
#include "gfx/gfx.h"


struct Point {
  double x;
  double y;
  Point(double x, double y) : x(x), y(y) {}
};

double dot(const Point& a, const Point& b) {
  return a.x * b.x + a.y * b.y;
}

double operator*(const Point& a, const Point& b) {
  return dot(a, b);
}

Point operator*(const Point& v, double s) {
  return Point(v.x * s, v.y * s);
}

Point operator/(const Point& v, double s) {
  return Point(v.x / s, v.y / s);
}

double len(const Point& v) {
  return sqrt(v.x*v.x + v.y*v.y);
}

double len2(const Point& v) {
  return v.x*v.x + v.y*v.y;
}

Point operator+(const Point& a, const Point& b) {
  return Point(a.x + b.x, a.y + b.y);
}

Point operator-(const Point& a, const Point& b) {
  return Point(a.x - b.x, a.y - b.y);
}

std::ostream& operator<<(std::ostream& os, const Point& v) {
  os << "(" << v.x << ", " << v.y << ")";
  return os;
}

bool operator!=(const Point& a, const Point& b) {
  return a.x != b.x || a.y != b.y; // TODO: use epsilon
}

Point normalize(const Point& v) {
  return v / len(v);
}

Point perpendicular(const Point& v) {
  return Point(-v.y, v.x);
}

double sign(double a) {
  if (a > 0.0) {
    return +1.0;
  }
  if (a < 0.0) {
    return -1.0;
  }
  return 0.0;
}

struct Line {
  Point a;
  Point b;
  Line(): a(0,0), b(0,0) {} 
  Line(const Point& a, const Point& b) : a(a), b(b) {}
  Point getVector() { return b-a; }
};

struct Polygon {
  std::vector<Point*> points;
  int textureId;
  float textureScale = 1.0;
  float textureX = 0.0;
  float textureY = 0.0;
  std::vector<Line> getLines() {
    std::vector<Line> lines;
    for (size_t i=0; i+1<points.size(); i++) {
      lines.push_back(Line(*(points[i]), *(points[i+1])));
    }
    return lines;
  }
};


struct Coin {
  Point position;
  Point velocity;
  Point acceleration;
  double rotation = 0.0;
  bool jump = false;
  bool onGround = false;
  Coin(int x, int y) : position(x, y), velocity(0, 0), acceleration(0, 0) {}
};

const int COIN_SIZE = 32;
const int POINT_SIZE = 3;
const double GRAVITY = 0.1;
const double GRAVITY_MAX = 0.1;

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
      textures.emplace_back("data/roof.png");
      textures.emplace_back("data/gray_wall.png");

      drawFilledPolygons = false;
      drawGrid = true;
      shouldSnapToGrid = true;
      loadLevel();
///      spawnCoin(200, 100);
    }

    void keyPressEvent(bool pressed, unsigned char key, unsigned short code) {
      if (pressed == true) {
        keyPress(key);
      }
      if (key == 229 || key == 225 || key == 16) {
        shiftKey = pressed;
      }
      if (key == 82 || key == 38) {
        jumpKey = pressed;
      }
      if (key == 80 || key == 37) {
        leftKey = pressed;
      }
      if (key == 79 || key == 39) {
        rightKey = pressed;
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

    void mouseWheel(int value) {
      if (value > 0) {
        mouseWheelUp();
      }
      else {
        mouseWheelDown();
      }
    }

    void joyButton(bool pressed, int button) {
      if (pressed == true) {
        jumpButtonPressed();
      } else {
        jumpButtonReleased();
      }
    }


    bool gameLoop() {
      mX = (mouse_x - scrollX)  * zoom;
      mY = (mouse_y - scrollY) * zoom;
      static int delay = 0;
//      if ((++delay)%10==0)
      {
        draw();
        update();
      }
      return true;
    }

  private:


// Input
///////////////////


    void mouseWheelUp() {
      zoom *= 1.25;
    }

    void mouseWheelDown() {
      zoom *= 0.75;
    }

    void mouseDownLeft() {
      if (newPolygon == nullptr) {
        // not creating new polygon
        Point* p = getPointAt(mX, mY);
        if (shiftKey) {
          if (p != nullptr) {
            if (selectedPoints.count(p) == 0) {
              selectedPoints.insert(p);
            }
          }
          else {
            selectingRectangle = true;
            selectingRectangleStart = Point(mX, mY);
          }
        }
        else {
          if (p == nullptr) {
            selectedPoints.clear();
          }
          if (p != nullptr) {
            if (selectedPoints.count(p) == 0 ) {
              selectedPoints.clear();
            }
            selectedPoints.insert(p);
            draggingPoints = true;
            draggingStart = Point(mX, mY);
          }
        }
      }
    }
    void mouseUpLeft() {
      if (newPolygon != nullptr) {
        addPointToPolygon(mX, mY, *newPolygon);
      }
      else {
        if (selectingRectangle) {
          selectingRectangle = false;
          for (Polygon* polygon: polygons) {
            for (Point* point: polygon->points) {
              float rectx0 = std::min(selectingRectangleStart.x, (double)mX);
              float recty0 = std::min(selectingRectangleStart.y, (double)mY);
              float rectx1 = std::max(selectingRectangleStart.x, (double)mX);
              float recty1 = std::max(selectingRectangleStart.y, (double)mY);
              if (point->x > rectx0 && point->x < rectx1 &&
                  point->y > recty0 && point->y < recty1) {
                selectedPoints.insert(point);
              }
            }
          }
        }
        if (draggingPoints == true) {
          draggingPoints = false;
        }
      }
    }

    void mouseDownRight() {}
    void mouseUpRight() {
      if (newPolygon != nullptr) {
        newPolygon->points.pop_back();
      }
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
      if (key == 127 || key == 46) { // delete
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
      if (key == '[') {
        Polygon* poly = getPolygonAt(mX, mY);
        poly->textureScale *= 0.5;
      }
      if (key == ']') {
        Polygon* poly = getPolygonAt(mX, mY);
        poly->textureScale *= 2.0;
      }
      if (key == 'j') {
        Polygon* poly = getPolygonAt(mX, mY);
        poly->textureY += 1;
      }
      if (key == 'k') {
        Polygon* poly = getPolygonAt(mX, mY);
        poly->textureY -= 1;
      }
      if (key == 'h') {
        Polygon* poly = getPolygonAt(mX, mY);
        poly->textureX -= 1;
      }
      if (key == 'l') {
        Polygon* poly = getPolygonAt(mX, mY);
        poly->textureX += 1;
      }
      if (key == 75 || key == 33) { // pg up
        Polygon* poly = getPolygonAt(mX, mY);
        poly->textureId++;
        if (poly->textureId == textures.size()) {
          poly->textureId = 0;
        }
      }
      if (key == 78 || key == 34) { // pg dn
        Polygon* poly = getPolygonAt(mX, mY);
        poly->textureId--;
        if (poly->textureId == -1) {
          poly->textureId = textures.size()-1;
        }
      }
//      if (key == 'l') {
//        loadLevel();
//      }
      if (key == 's') {
        saveLevel();
      }
    }

    double getJoyValue() {
      // std::cout << "input:   " << joy_x;
      int reduced = joy_x / 1024;
      // std::cout << "reduced: " << reduced << std::endl;
      double scaled = reduced / 30.0;
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

    void jumpButtonPressed() {
      std::cout << "jump" << std::endl;
      if (coin != nullptr) {
        coin->jump = true;
      }
    }

    void jumpButtonReleased() {
      std::cout << "no jump" << std::endl;
      if (coin != nullptr) {
        coin->jump = false;
      }
    }

// Level serialization
///////////////////


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
      std::ofstream file("data/level.txt");
      file << pointIds.size() << std::endl;
      for (const auto& p: pointsById) {
        file << p->x << " " << p->y << std::endl;
      }
      file << polygons.size() << std::endl;
      for (Polygon* polygon: polygons) {
        file << polygon->textureId << " ";
        file << polygon->textureScale << " ";
        file << polygon->textureX << " ";
        file << polygon->textureY << " ";
        file << polygon->points.size();
        for (Point* point: polygon->points) {
          file << " " << pointIds[point];
        }
        file << std::endl;
      }
    }

    void loadLevel() {
      std::ifstream file("data/level.txt");
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
        float textureScale;
        float textureX;
        float textureY;
        file >> textureId >> textureScale >> textureX >> textureY >> numPoints;
        polygon->textureId = textureId;
        polygon->textureScale = textureScale;
        polygon->textureX = textureX;
        polygon->textureY = textureY;
        for (int j=0; j<numPoints; j++) {
          int pointId;
          file >> pointId;
          polygon->points.push_back(points[pointId]);
        }
        polygons.push_back(polygon);
      }
    }


// Level editting
///////////////////


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


// Polygon math
///////////////////


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

    bool sideOfLine(double x, double y, Point* a, Point* b) {
      // std::cout << "checking side " << x << ", " << y << " of line "
      //   << a->x << ", " << a->y << " - " << b->x << ", " << b->y;

      double dy = (b->y - a->y);
      double dx = (b->x - a->x);
      if (dx == 0) {
        if (a->y > b->y) {
          return x > a->x;
        }
        else {
          return x < a->x;
        }
      }
      double direction = dy/dx;
      double position = a->y - direction*a->x;

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

    bool isInPolygon(double x, double y, const Polygon& poly) {
      // std::cout << "checking if " << x << ", " << y << " is in polygon" << std::endl;
      for (size_t i=0; i+1<poly.points.size(); i++) {
        bool side = sideOfLine(x, y, poly.points[i], poly.points[i+1]);
        if (side == false) {
          return false;
        }
      }
      return true;
    }

    Polygon* getPolygonAt(const Point& p) { return getPolygonAt(p.x, p.y); }
    Polygon* getPolygonAt(double x, double y) {
      for (Polygon* polygon: polygons) {
        if (isInPolygon(x, y, *polygon)) {
          return polygon;
        }
      }
      return nullptr;
    }

// Physics
///////////////////

    Point project(Point what, Point where) {
      double magn = (what * where) / len2(where);
      return where * magn;
    }

    double lineIntersection(const Point& p0, const Point& v, const Point& pa, const Point& pb) {
      Point n = normalize(perpendicular(pb - pa));
      Point o = p0 + (n * COIN_SIZE); // intersection on sphere
      double t = (n * (pa - o)) / (n * v);
      if (t >= 0.0 && t <= 1.0) {
        Point o2 = o + v * t;
        double oa = len(o2 - pa);
        double ob = len(o2 - pb);
        double ab = len(pb - pa);
        if (oa < ab && ob < ab) {
          return t;
        }
      }
      return FLT_MAX;
    }

    float pointIntersection(const Point& p0, const Point& v, const Point& pa) {
      Point q = p0 - pa;
      float a = v * v;
      float b = 2.0 * (q * v);
      float c = (q * q) - COIN_SIZE * COIN_SIZE;
      float delta = b * b - 4.0 * a * c;
      if (delta >= 0) {
        float t1 = (-b + sqrt(delta)) / (2.0 * a);
        float t2 = (-b - sqrt(delta)) / (2.0 * a);
        if (t1 >= 0 && t1 <= 1.0) {
          if (t2 >= 0 && t2 <= 1.0) {
            if (t2 < t1) {
              return t2;
            }
            else {
              return t1;
            }
          }
          return t1;
        }
        if (t2 >= 0 && t2 <= 1.0) {
          return t2;
        }
      }
      return FLT_MAX;
    }

    std::tuple<Point, Point> checkCollisions(const Point& p0, const Point& v) {
      double minT = FLT_MAX;
      Line minLine;
      Point* minPoint = nullptr;
      for (Polygon* polygon: polygons) {
        auto lines = polygon->getLines();
        for (auto& line: lines) {
          double t = lineIntersection(p0, v, line.a, line.b);
          if (t < minT && t >= 0.0) {
            minT = t;
            minLine = line;
            minPoint = nullptr;
          }
          t = pointIntersection(p0, v, line.a);
          if (t< 1.0 && t < minT) {
            minT = t;
            minLine = line;
            minPoint = &minLine.a;
          }
          t = pointIntersection(p0, v, line.b);
          if (t< 1.0 && t < minT) {
            minT = t;
            minLine = line;
            minPoint = &minLine.b;
          }
        }
      }
      static const double epsilon = 0.1; // !!
      minT = minT - epsilon;
      if (minT <= 1.0) {
        Point newP = p0 + v * minT;
        Point slidePlane(0,0);
        if (minPoint == nullptr) {
          slidePlane = minLine.a - minLine.b;
        } else {
          std::cout << "Point collision" << std::endl;
          slidePlane = perpendicular(newP - (*minPoint));
        }
        Point newV = project(v*(1.0-minT), slidePlane);
        return checkCollisions(newP, newV);
      }
      return std::make_tuple(p0, v);
    }

    Point clipVelocity(const Point& p0, const Point& v) {
      Point newP(0,0);
      Point newV(0,0);
      std::tie(newP, newV) = checkCollisions(p0, v);
      return (newP + newV) - p0;
    }

    void applyVelocity(Coin& coin) {
      Point v = coin.velocity + coin.acceleration;
      coin.velocity = clipVelocity(coin.position, v);
      coin.position = coin.position + coin.velocity;
    }

    void updateCoin(Coin& coin) {
      coin.acceleration = {0.0, 0.0};
      if (jumpKey) {
        coin.jump = true;
      }
      if (coin.onGround && coin.jump) {
        coin.acceleration.y = -GRAVITY_MAX*33;
        coin.jump = false;
      }
      else {
        coin.acceleration.y = GRAVITY_MAX;
      }
      if (rightKey) {
        coin.acceleration.x = +0.036;
      }
      else if (leftKey) {
        coin.acceleration.x = -0.036;
      }
      else {
        double joyValue = getJoyValue();
        coin.acceleration.x = joyValue * 0.03;
      }
      applyVelocity(coin);
    }

    Point snapToGrid(const Point& p) {
      if (shouldSnapToGrid) {
        return Point(round(p.x/gridSize)*gridSize, round(p.y/gridSize)*gridSize);
      }
      return p;
    }

    void update() {
      if (scrolling) {
        scrollX += (mouse_x - scrollStartX);
        scrollY += (mouse_y - scrollStartY);
        scrollStartX = mouse_x;
        scrollStartY = mouse_y;
      }
      if (draggingPoints) {
        Point mousePosition(mX, mY);
        mousePosition = snapToGrid(mousePosition);
        if (snapToGrid(mousePosition - draggingStart) != Point(0,0)) {
          for (Point* point: selectedPoints) {
            *point = snapToGrid(*point + (mousePosition - draggingStart));
          }
          draggingStart = mousePosition;
        }
      }
      if (coin != nullptr) {
        updateCoin(*coin);
      }
    }


// Drawing
///////////////////


    void drawPoint(const Point& point) {
      static const int SIZE = POINT_SIZE;
      prim.drawRectangle(point.x * zoom - SIZE, point.y * zoom - SIZE,
                         point.x * zoom + SIZE, point.y * zoom + SIZE);
    }

    void drawLine(const Point& a, const Point& b) {
      prim.drawLine(a.x * zoom, a.y * zoom,
                    b.x * zoom, b.y * zoom);
    }

    void drawPolygon(const Polygon& polygon) {
      if (drawFilledPolygons) {
        std::vector<float> positions;
        for (Point* point: polygon.points) {
          positions.push_back(point->x * zoom);
          positions.push_back(point->y * zoom);
        }
        glBindTexture(GL_TEXTURE_2D, textures[polygon.textureId].getTexture());
        prim.drawConvexPolygon(positions, polygon.textureScale / zoom, polygon.textureX * zoom, polygon.textureY * zoom);
      }
      else {
        for (const auto& point: polygon.points) {
          if (selectedPoints.count(point)) {
            prim.setColor(0,0,1,1);
          } else {
            prim.setColor(1,1,1,1);
          }
          drawPoint(*point);
        }
        for (size_t i=0; (i+1)<polygon.points.size(); i++) {
          prim.setColor(1,1,1,1);
          drawLine(*polygon.points[i], *polygon.points[i+1]);
          {
            Point v = *polygon.points[i+1] - *polygon.points[i];
            Point n = perpendicular(v) / len(v);
            Point mid = *polygon.points[i] + (v * 0.5);
            drawLine(mid, mid + n*10);
          }
        }
      }
    }

    void draw() {
      glClear(GL_COLOR_BUFFER_BIT);
      if (drawGrid) {
        prim.setColor((101/255.0)*0.9, (184/255.0)*0.9, (227/225.0)*0.9, 1.0);
        prim.setColor(0,0,0, 0.06);
        for (int y=0; y<screen_h/gridSize; y++) {
          prim.drawLine(0,gridSize*y, screen_w, gridSize*y);
        }
        for (int x=0; x<screen_w/gridSize; x++) {
          prim.drawLine(gridSize*x, 0, gridSize*x, screen_h);
        }
      }
      if (coin != nullptr) {
        prim.setScroll((screen_w/2)-coin->position.x,
                       (screen_h/2)-coin->position.y);
      }
      else {
        prim.setScroll(scrollX, scrollY);
    //    prim.setScale(zoom);
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
        if (!drawFilledPolygons) {
          prim.drawLine(coin->position.x, coin->position.y,
                        coin->position.x + coin->velocity.x * 30,
                        coin->position.y + coin->velocity.y * 30
              );
        }
      }
      if (selectingRectangle) {
        prim.drawLine(selectingRectangleStart.x, selectingRectangleStart.y, mX, selectingRectangleStart.y);
        prim.drawLine(selectingRectangleStart.x, selectingRectangleStart.y, selectingRectangleStart.x, mY);
        prim.drawLine(mX, mY, selectingRectangleStart.x, mY);
        prim.drawLine(mX, mY, mX, selectingRectangleStart.y);
      }
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
//    Point* currentPoint = nullptr;

    std::set<Point*> selectedPoints;
    bool draggingPoints = false;
    Point draggingStart = Point(0,0);
    bool shiftKey = false;
    Point selectingRectangleStart = Point(0,0);
    bool selectingRectangle = false;
    bool drawGrid = false;
    bool shouldSnapToGrid = false;
    float gridSize = 10.0;

    std::vector<Image> textures;
    double scrollX = 0;
    double scrollY = 0;
    float zoom = 1.0;
    bool scrolling = false;
    int scrollStartX;
    int scrollStartY;
    int mX;
    int mY;
    bool leftKey = false;
    bool rightKey = false;
    bool jumpKey = false;
};






Editor* editor = nullptr;


void key_press(bool pressed, unsigned char key, unsigned short code) {
  editor->keyPressEvent(pressed, key, code);
}

void mouse_button(bool pressed, int button, int x, int y ) {
  editor->mouseButton(pressed, button);
}

void mouse_wheel(int value) {
  editor->mouseWheel(value);
}

void joy_button(bool pressed, int button) {
  editor->joyButton(pressed, button);
}

bool gameLoop() {
  return editor->gameLoop();
}

void gameInit() {
  const int w = 1600;
  const int h  = (w/16.0)*9.0;
  createWindow(w, h, "Currency - Editor");
  glViewport(0, 0, screen_w, screen_h);
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glClearColor(101/255.0 * 0.5,
               184/255.0 * 0.5,
               227/225.0 * 0.5, 1.0);
  editor = new Editor();
}

void gameCleanup() {
  delete editor;
}


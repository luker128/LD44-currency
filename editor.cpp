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

struct Edge {
  Point* a;
  Point* b;
  bool shared;
  Edge(Point& a, Point& b) : a(&a), b(&b), shared(false) {
    std::cout << "Creating edge " << this << " of points " << &a << " and " << &b << std::endl;
    std::cout << "  positions: " << a << ", " << b << std::endl;
  }
  Point getVector() { return (*b)-(*a); }
  bool isExternal() { return !shared; }
};

struct Triangle {
  std::vector<Edge*> edges;
  Triangle() {}
};

struct Face {
  std::vector<Triangle*> triangles;
  int textureId;
  float textureScale = 1.0;
  float textureX = 0.0;
  float textureY = 0.0;

  std::vector<Line> getLines() {
    std::vector<Line> lines;
    for (auto triangle: triangles) {
      for (auto edge: triangle->edges) {
        if (edge->isExternal()) {
          lines.push_back(Line(*(edge->a), *(edge->b)));
        }
      }
    }
    return lines;
  }

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

    struct NewTriangle {
      std::vector<Point*> points;
      NewTriangle(Point* p = nullptr) {
        if (p != nullptr) {
          points.push_back(p);
        }
      }
    };

    Editor(const Editor&) = delete;

  public:

    Editor() :
      fontSheet("data/font.png", 16, 16),
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

      drawTextures = false;
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

    double signedArea(Point& a, Point& b, Point& c) {
      return 0.5 * (-1*a.y*b.x + a.x*b.y +a.y*c.x -b.y*c.x -a.x*c.y +b.x*c.y);
    }

    void addTriangleToLevel(const NewTriangle& trianglePoints) {
      Face* newFace = new Face();
      Triangle* triangle = new Triangle();

      {
        Point& a = *trianglePoints.points[0];
        Point& b = *trianglePoints.points[1];
        Point& c = *trianglePoints.points[2];
        double sa = signedArea(a, b, c);
        if (sa < 0) {
          auto tmp = b;
          b = c;
          c = tmp;
        }
        triangle->edges.push_back(new Edge(a, b));
        triangle->edges.push_back(new Edge(b, c));
        triangle->edges.push_back(new Edge(c, a));
      }

      newFace->triangles.push_back(triangle);
      faces.push_back(newFace);
    }

    void mouseUpLeft() {
      if (newTriangle != nullptr) {
        Point* point = getOrCreatePoint(mX, mY);
        newTriangle->points.push_back(point);
        if (newTriangle->points.size() == 3) {
          addTriangleToLevel(*newTriangle);
          delete newTriangle;
          newTriangle = nullptr;
        }
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
      Face* face = getFaceAt(Point(mX, mY));
      if (key == 127 || key == 46) { // delete
        delete coin;
        coin = nullptr;
      }
      if (key == 32) { // SPACE
        if (newTriangle == nullptr) {
          Point* point = getPointAt(mX, mY);
          newTriangle = new NewTriangle(point);
        }
      }
      if (key == 'f') {
        drawTextures = !drawTextures;
      }
      if (key == '\r') {
        spawnCoin(mX, mY);
      }
      if (key == '[') {
        face->textureScale *= 0.5;
      }
      if (key == ']') {
        face->textureScale *= 2.0;
      }
      if (key == 'j') {
        face->textureY += 1;
      }
      if (key == 'k') {
        face->textureY -= 1;
      }
      if (key == 'h') {
        face->textureX -= 1;
      }
      if (key == 'l') {
        face->textureX += 1;
      }
      if (key == 75 || key == 33) { // pg up
        face->textureId++;
        if (face->textureId == textures.size()) {
          face->textureId = 0;
        }
      }
      if (key == 78 || key == 34) { // pg dn
        face->textureId--;
        if (face->textureId == -1) {
          face->textureId = textures.size()-1;
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
      for (Face* face: faces) {
        for (Triangle* triangle: face->triangles) {
          for (Edge* edge: triangle->edges) {
            Point* point = edge->a;
            if (pointIds.find(point) == pointIds.end()) {
              pointIds[point] = count;
              count++;
              pointsById.push_back(point);
            }
          }
        }
      }
      std::ofstream file("data/level.txt");
      file << pointIds.size() << std::endl;
      for (const auto& p: pointsById) {
        file << p->x << " " << p->y << std::endl;
      }
      file << faces.size() << std::endl;
      for (Face* face: faces) {
        file << face->textureId << " ";
        file << face->textureScale << " ";
        file << face->textureX << " ";
        file << face->textureY << " ";
        file << face->triangles.size();
        for (Triangle* triangle: face->triangles) {
          for (Edge* edge: triangle->edges) {
            Point* point = edge->a;
            file << " " << pointIds[point];
          }
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
      int numFaces;
      file >> numFaces;
      std::map<std::pair<Point*, Point*>, Edge*> edgeMap;
      for (int i=0; i<numFaces; i++) {
        int textureId;
        int numTriangles;
        float textureScale;
        float textureX;
        float textureY;
        file >> textureId >> textureScale >> textureX >> textureY >> numTriangles;
        std::vector<Point*> pointsInPoly;
        for (int j=0; j<numTriangles*3; j++) {
          int pointId;
          file >> pointId;
          pointsInPoly.push_back(points[pointId]);
        }
        Face* face = new Face;
        face->textureId = textureId;
        face->textureScale = textureScale;
        face->textureX = textureX;
        face->textureY = textureY;
        for (int triId=0; triId<numTriangles; triId++) {
          Triangle* triangle = new Triangle();
          for (int pointId=0; pointId<3; pointId++) {
            Point* a = pointsInPoly[triId*3 + pointId];
            Point* b;
            if (pointId == 2) {
              b = pointsInPoly[triId*3]; // wrap back to start
            }
            else {
              b = pointsInPoly[triId*3 + pointId + 1];
            }
            auto key = std::make_pair(a, b);
            Edge* edge;
            auto it = edgeMap.find(key);
            if (it == edgeMap.end()) {
              edge = new Edge(*a, *b);
              edgeMap.insert(std::make_pair(key, edge));
            }
            else {
              edge = it->second;
              edge->shared = true;
            }
            triangle->edges.push_back(edge);
          }
          face->triangles.push_back(triangle);
        }
        faces.push_back(face);
      }
    }


// Level editting
///////////////////

    Point* getOrCreatePoint(int x, int y) {
      Point* oldPoint = getPointAt(x, y);
      if (oldPoint != nullptr) {
        return oldPoint;
      }
      return new Point(x, y);
    }


// Polygon math
///////////////////


    Point* getPointAt(int x, int y) {
      for (auto& face: faces) {
        for (auto& triangle: face->triangles) {
          for (auto& edge: triangle->edges) {
            Point* point = edge->a;
            int dx = abs(x - point->x);
            int dy = abs(y - point->y);
            if (std::max(dx, dy) <= POINT_SIZE) {
              return point;
            }
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

    Face* getFaceContaining(const Triangle& t) {
      for (auto face: faces) {
        for (auto triangle: face->triangles) {
          if (triangle == &t) {
            return face;
          }
        }
      }
      return nullptr;
    }

    bool sideOfEdge(const Point& point, const Edge& edge) {
      return sideOfLine(point.x, point.y, edge.a, edge.b);
    }

    bool isInTriangle(const Point& point, const Triangle& triangle) {
      for (auto edge: triangle.edges) {
        bool side = sideOfEdge(point, *edge);
        if (side == false) {
          return false;
        }
      }
      return true;
    }

    Triangle* getTriangleAt(const Point& p) {
      for (auto face: faces) {
        for (auto triangle: face->triangles) {
          if (isInTriangle(p, *triangle)) {
            return triangle;
          }
        }
      }
      return nullptr;
    }

    Face* getFaceAt(const Point& point) { // TODO: inefficient, rewrite
      Triangle* triangle = getTriangleAt(point);
      if (triangle == nullptr) {
        return nullptr;
      }
      return getFaceContaining(*triangle);
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

    void print(int start_x, int start_y, const std::string& text, float size=1.0) {
      int x = start_x - 8;
      int y = start_y - 8;
      const float kern = 0.8;
      for (char c: text) {
        if (c == '\n') {
          x = start_x - 8;
          y += 20;
        }
        else {
          fontSheet.drawSpriteScaled(x, y, c, size);
          x += fontSheet.getFrameWidth() * kern * size;
        }
      }
    }

    void drawPoint(const Point& point) {
      static const int SIZE = POINT_SIZE;
      prim.drawRectangle(point.x * zoom - SIZE, point.y * zoom - SIZE,
                         point.x * zoom + SIZE, point.y * zoom + SIZE);
    }

    void drawLine(const Point& a, const Point& b) {
      prim.drawLine(a.x * zoom, a.y * zoom,
                    b.x * zoom, b.y * zoom);
    }

    void drawFaceTextured(const Face& face) {
      std::vector<float> positions;
      for (auto triangle: face.triangles) {
        for (auto edge: triangle->edges) {
          Point* point = edge->a;
          positions.push_back(point->x * zoom);
          positions.push_back(point->y * zoom);
        }
      }
      glBindTexture(GL_TEXTURE_2D, textures[face.textureId].getTexture());
      prim.drawConvexPolygon(positions, face.textureScale / zoom, face.textureX * zoom, face.textureY * zoom);
    }

    void drawFaceWireframe(const Face& face) {
      std::set<Point*> pointsToDraw;
      for (auto triangle: face.triangles) {
        for (auto edge: triangle->edges) {
          drawLine(*edge->a, *edge->b);
          pointsToDraw.insert(edge->a);
          pointsToDraw.insert(edge->b);
          Point v = edge->getVector();
          Point n = perpendicular(v) / len(v);
          Point mid = *edge->a + (v * 0.5);
          drawLine(mid, mid + n*10);
        }
      }
      for (auto point: pointsToDraw) {
        if (selectedPoints.count(point)) {
          prim.setColor(0,0,1,1);
        } else {
          prim.setColor(1,1,1,1);
        }
        drawPoint(*point);
      }
    }

    void drawFace(const Face& face) {
      if (drawTextures) {
        drawFaceTextured(face);
      }
      drawFaceWireframe(face);
    }

    void drawNewTriangle(NewTriangle* triangle) {
      if (triangle != nullptr && triangle->points.size() > 0) {
        for (int i=0; i<triangle->points.size()-1; i++) {
          drawLine(*triangle->points[i], *triangle->points[i+1]);
        }
        drawLine(*triangle->points[triangle->points.size()-1], Point(mX, mY));
      }
    }

    std::string getModeName() {
      if (newTriangle != nullptr) {
        return "Creating triangle";
      }
      else {
        return "Idle";
      }
    }

    void printStatus() {
      Triangle* triangle = getTriangleAt(Point(mX, mY));
      Face* polygon = nullptr;
      if (triangle != nullptr) {
        polygon = getFaceContaining(*triangle);
      }
      Point* point = getPointAt(mX, mY);
      std::ostringstream os;
      os << "Mode: " << getModeName() << "\n";
      os << "Face: " << polygon << "\n";
      if (polygon) {
        os << "  textureId: " << polygon->textureId << "\n";
        os << "  textureScale: " << polygon->textureScale << "\n";
        os << "  textureX/Y: " << Point(polygon->textureX, polygon->textureY) << "\n";
      }
      os << "Point:" << point << "\n";
      if (point) {
        os << "  " << *point << "\n";
      }
      print(32,32, os.str());
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
      for (Face* face: faces) {
        drawFace(*face);
      }
      prim.setColor(1,1,0,1);
      if (newTriangle != nullptr) {
        drawNewTriangle(newTriangle);
      }
      if (coin != nullptr) {
        if (coin->velocity.x != 0) {
          coin->rotation += coin->velocity.x / COIN_SIZE;
        }
        //coinImage.drawSpriteRotated(coin->position.x+scrollX, coin->position.y+scrollY, 0, coin->rotation);
        coinImage.drawSpriteRotated(screen_w / 2, screen_h /2, 0, coin->rotation);
        if (!drawTextures) {
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
      printStatus();
    }

    void spawnCoin(int x, int y) {
      std::cout << "Spawning coin at " << x << ", " << y << std::endl;
      delete coin;
      coin = new Coin(x,y);
    }

    SpriteSheet fontSheet;

    std::vector<Face*> faces;
    NewTriangle* newTriangle = nullptr;

    Coin* coin = nullptr;
    bool drawTextures = false;
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


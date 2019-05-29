#pragma once
#include <vector>
#include "util.h"

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

struct Level {
  std::vector<Face*> faces;
};

void saveLevel(Level& level);
void loadLevel(Level& level);




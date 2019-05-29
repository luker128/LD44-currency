#pragma once
#include "util.h"
#include "level.h"

const int COIN_SIZE = 32;
const int POINT_SIZE = 3;
const double GRAVITY = 0.1;
const double GRAVITY_MAX = 0.1;

struct Coin {
  Point position;
  Point velocity;
  Point acceleration;
  double rotation = 0.0;
  bool jump = false;
  bool onGround = false;
  Coin(int x, int y) : position(x, y), velocity(0, 0), acceleration(0, 0) {}
};


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

    std::tuple<Point, Point> checkCollisions(const Point& p0, const Point& v, std::vector<Polygon*>& polygons) {
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
        return checkCollisions(newP, newV, polygons);
      }
      return std::make_tuple(p0, v);
    }

    Point clipVelocity(const Point& p0, const Point& v, std::vector<Polygon*>& polygons) {
      Point newP(0,0);
      Point newV(0,0);
      std::tie(newP, newV) = checkCollisions(p0, v, polygons);
      return (newP + newV) - p0;
    }

    void applyVelocity(Coin& coin, std::vector<Polygon*>& polygons) {
      Point v = coin.velocity + coin.acceleration;
      coin.velocity = clipVelocity(coin.position, v, polygons);
      coin.position = coin.position + coin.velocity;
    }

    void updateCoin(Coin& coin, std::vector<Polygon*>& polygons, bool leftKey, bool rightKey, bool jumpKey, double joyValue) {
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
        coin.acceleration.x = joyValue * 0.03;
      }
      applyVelocity(coin, polygons);
    }



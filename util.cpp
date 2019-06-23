#include <cmath>
#include "util.h"

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

Point& operator+=(Point& lhs, const Point& rhs) {
  lhs.x += rhs.x;
  lhs.y += rhs.y;
  return lhs;
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



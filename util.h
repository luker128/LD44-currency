#pragma once
#include <iostream>

struct Point {
  double x;
  double y;
  Point(double x, double y) : x(x), y(y) {}
};

double dot(const Point& a, const Point& b);
double operator*(const Point& a, const Point& b);
Point operator*(const Point& v, double s);
Point operator/(const Point& v, double s);
Point& operator+=(Point& lhs, const Point& rhs);
double len(const Point& v);
double len2(const Point& v);
Point operator+(const Point& a, const Point& b);
Point operator-(const Point& a, const Point& b);
std::ostream& operator<<(std::ostream& os, const Point& v);
bool operator!=(const Point& a, const Point& b);
Point normalize(const Point& v);
Point perpendicular(const Point& v);
double sign(double a);



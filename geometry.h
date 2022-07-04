#pragma once
#ifndef GEOMETRY_H_SENTRY
#define GEOMETRY_H_SENTRY

#include <cmath>
#include <vector>
#include <memory>

struct Point2d {
  int y, x;
  void add_(Point2d other) { y += other.y; x += other.x; }
  Point2d operator-() const { return Point2d{.y = -y, .x = -x}; }
  Point2d operator+(Point2d other) const { return Point2d{.y = y + other.y, .x = x + other.x}; }
  Point2d operator-(Point2d other) const { return (*this) + (-other); }
  Point2d operator*(float scalar) const { return Point2d{.y = (int)(scalar*y), .x = (int)(scalar*x)}; }
  int dot(Point2d other) const { return y*other.y + x*other.x; }
  float angle(Point2d other) const;
  float length() const { return sqrt(this->dot(*this)); }
  Point2d rot90() const { return Point2d{.y = -x, .x = y}; }
};
Point2d operator*(float scalar, Point2d point);

class Transform{
public:
  virtual Point2d operator()(Point2d x) const = 0;
  virtual ~Transform() {}
};
class TransformChain {
  std::vector<std::unique_ptr<Transform>> transforms;
public:
  void append(std::unique_ptr<Transform>);
  Point2d backward(Point2d point) const;
};
class Translation : public Transform {
  Point2d shift;
public:
  Translation(Point2d shift): shift(shift) {}
  Point2d operator()(Point2d x) const override;
};
class Rotation : public Transform {
  float c, s; // cos and sin
public:
  Rotation(float rot): c(cos(rot)), s(sin(rot)) {}
  Point2d operator()(Point2d x) const override;
};

struct Box2d {
  Point2d lt, rb;

  void intersect_(const Box2d& other);
};

// correct points layout is not ensured
struct Rectangle {
  Point2d points[4];

  Rectangle(Box2d);

  // returns inverse
  Translation translate_(Point2d shift);
  // returns inverse
  Rotation rotate_(float rot);

  bool contains(Point2d point) const;
};

// 128 bits is seemingly enough to use reference??
Box2d boundingBox2d(const Box2d& square, float rot);

Box2d boundingBox2d(const Rectangle& rect);

// TODO enabel_if n_points > 0
template <unsigned n_points>
Box2d boundingBox2d(const Point2d points[n_points]);



#endif // GEOMETRY_H_SENTRY

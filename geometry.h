#pragma once
#ifndef GEOMETRY_H_SENTRY
#define GEOMETRY_H_SENTRY

#include <cmath>
#include <vector>
#include <memory>
#include <iosfwd>

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

struct CollisionShape {
protected:
  enum struct Type {
    unknown, triangle, rectangle, sphere,
  } type;
public:
  virtual bool collides(const CollisionShape& other, Type=Type::unknown) const = 0;
  virtual ~CollisionShape() {}
};
namespace collision_shape {
class Error {};
class SRError : public Error {
  const char *type;
public:
  SRError(const char* t): type(t) {}
  const char *what() const { return type; }
};
std::unique_ptr<CollisionShape> readFromStream(std::istream&);
} // collision_shape
// correct points layout is not ensured
struct Triangle;
struct Sphere;
struct Rectangle : CollisionShape {
private:
  static constexpr Type type = Type::rectangle;
public:
  Point2d points[4];

  Rectangle(Box2d);

  // returns inverse
  Translation translate_(Point2d shift);
  // returns inverse
  Rotation rotate_(float rot);

  bool contains(Point2d point) const;

  bool collides(const CollisionShape& other, Type=Type::unknown) const override;
  bool collides(const Rectangle& other) const;
  bool collides(const Triangle& triangle) const;
  bool collides(const Sphere& sphere) const;
};

struct Triangle : CollisionShape {
private:
  static constexpr Type type = Type::triangle;
public:
  Point2d points[3]; // counter-clockwise

  bool contains(Point2d point) const;

  bool collides(const CollisionShape& other, Type=Type::unknown) const override;
  bool collides(const Rectangle& rectangle) const;
  bool collides(const Triangle& other) const;
  bool collides(const Sphere& sphere) const;
};

struct Sphere : CollisionShape {
  using spherical = void*;
private:
  static constexpr Type type = Type::sphere;
public:
  Point2d center;
  float radius;

  bool contains(Point2d point) const;
  
  // bool collides(const Rectangle& rectangle) const;
  // bool collides(const Triangle& triangle) const;
  bool collides(const CollisionShape& other, Type=Type::unknown) const override;
  bool collides(const Sphere& other) const;
  template <class T>
  bool collides(const T& shape) const;
};

// 128 bits is seemingly enough to use reference??
Box2d boundingBox2d(const Box2d& square, float rot);

Box2d boundingBox2d(const Rectangle& rect);

// TODO enabel_if n_points > 0
template <unsigned n_points>
Box2d boundingBox2d(const Point2d points[n_points]) {
  static_assert(n_points > 0);
  int minx = points[0].x;
  int maxx = points[0].x;
  int miny = points[0].y;
  int maxy = points[0].y;
  for (const Point2d *point = points; point != points + n_points; ++point) {
    if (point->x < minx) {
      minx = point->x;
    } else if (point->x > maxx) {
      maxx = point->x;
    }
    if (point->y < miny) {
      miny = point->y;
    } else if (point->y > maxy) {
      maxy = point->y;
    }
  }
  return Box2d{
    .lt = Point2d{
      .y = miny, .x = minx,
    },
    .rb = Point2d{
      .y = maxy, .x = maxx,
    },
  };
}

template <unsigned l_points, unsigned r_points>
bool collides(const Point2d lhs[l_points], const Point2d rhs[r_points]) {
  // might need optimization

  bool found_pos_only = true, found_neg_only = true;
  for (unsigned i = 0; i < l_points; ++i) {
    Point2d ldiff90 = (lhs[(i+1)%l_points] - lhs[i]).rot90();
    for (unsigned j = 0; j < r_points; ++j) {
      if (ldiff90.dot(rhs[i] - lhs[i]) > 0) {
        found_neg_only = false;
      } else {
        found_pos_only = false;
      }
    }
  }

  found_pos_only = true;
  found_neg_only = true;
  if (found_pos_only || found_neg_only) {
    for (unsigned j = 0; j < r_points; ++j) {
      Point2d rdiff90 = (rhs[(j+1)%r_points] - rhs[j]).rot90();
      for (unsigned i = 0; i < l_points; ++i) {
        if (rdiff90.dot(lhs[i] - rhs[i]) > 0) {
          found_neg_only = false;
        } else {
          found_pos_only = false;
        }
      }
    }
    return found_pos_only || found_neg_only;
  } else {
    return false;
  }
}

// assume counter-clockwise shape
template <class T>
bool Sphere::collides(const T& shape) const {
  // might need optimization

  // check distance to edges
  using Point = decltype(shape.points[0]);
  unsigned n_points = sizeof(shape.points) / sizeof(Point);
  for (unsigned i = 0; i < n_points; ++i) {
    Point diff90 = (shape.points[(i+1)%n_points] - shape.points[i]).rot90();
    Point diff_center = center - shape.points[i];
    if (diff90.dot(diff_center) / (diff90.length() * diff_center.length()) < radius) {
      return true;
    }
  }

  // check center
  if (shape.contains(center)) {
    return true;
  }

  return false;
}

#endif // GEOMETRY_H_SENTRY

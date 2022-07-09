#pragma once
#ifndef GEOMETRY_H_SENTRY
#define GEOMETRY_H_SENTRY

#include <cmath>
#include <vector>
#include <memory>
#include <iosfwd>

template <class F = int>
struct TPoint2d {
  F y, x;
  void add_(TPoint2d other) { y += other.y; x += other.x; }
  TPoint2d operator-() const { return TPoint2d{.y = -y, .x = -x}; }
  TPoint2d operator+(TPoint2d other) const { return TPoint2d{.y = y + other.y, .x = x + other.x}; }
  TPoint2d operator-(TPoint2d other) const { return (*this) + (-other); }
  TPoint2d operator*(float scalar) const { return TPoint2d{.y = (F)(scalar*y), .x = (F)(scalar*x)}; }
  int dot(TPoint2d other) const { return y*other.y + x*other.x; }
  float angle(TPoint2d other) const {
    float normalizer = 1.0/(this->length() * other.length());
    float c = this->dot(other) * normalizer;
    float s = this->rot90().dot(other) * normalizer;
    float ang_mag = acos(c);
    return (1 - 2*(s > 0)) * ang_mag;
  }
  float length() const { return sqrt(this->dot(*this)); }
  TPoint2d rot90() const { return TPoint2d{.y = -x, .x = y}; }
};
using Point2d = TPoint2d<int>;
using Point2dF = TPoint2d<float>;
Point2d operator*(float scalar, Point2d point);
Point2dF operator*(float scalar, Point2dF point);

template <class F>
class TTransform{
public:
  virtual TPoint2d<F> operator()(TPoint2d<F> x) const = 0;
  virtual ~TTransform() {}
};
template <class F>
class TTransformChain {
  std::vector<std::unique_ptr<TTransform<F>>> transforms;
public:
  void append(std::unique_ptr<TTransform<F>> link) { transforms.push_back(std::move(link)); }
  TPoint2d<F> backward(TPoint2d<F> point) const {
    for (auto iter = transforms.rbegin(); iter != transforms.rend(); ++iter) {
      point = (**iter)(point);
    }
    return point;
  }
  TPoint2d<F> forward(TPoint2d<F> point) const {
    for (auto iter = transforms.begin(); iter != transforms.end(); ++iter) {
      point = (**iter)(point);
    }
    return point;
  }
};
template <class F>
class TTranslation : public TTransform<F> {
  TPoint2d<F> shift;
public:
  TTranslation(TPoint2d<F> shift): shift(shift) {}
  TPoint2d<F> operator()(TPoint2d<F> point) const override {
    return point + shift;
  }
};
template <class F>
class TRotation : public TTransform<F> {
  float c, s; // cos and sin
public:
  TRotation(float rot): c(cos(rot)), s(sin(rot)) {}
  TPoint2d<F> operator()(TPoint2d<F> point) const override {
  return TPoint2d<F>{
    .y = (F)(-point.x*s + point.y*c),
    .x = (F)(point.x*c + point.y*s),
  };
}
};

using Transform = TTransform<int>;
using TransformChain = TTransformChain<int>;
using Translation = TTranslation<int>;
using Rotation = TRotation<int>;
using TransformF = TTransform<float>;
using TransformChainF = TTransformChain<float>;
using TranslationF = TTranslation<float>;
using RotationF = TRotation<float>;

struct Box2d {
  Point2d lt, rb;

  void intersect_(const Box2d& other);
  bool contains(Point2d point) const;
};

struct CollisionShape {
protected:
  enum struct Type {
    unknown, triangle, rectangle, sphere,
  } type;
public:
  virtual bool collides(const CollisionShape& other, Type=Type::unknown) const = 0;
  virtual std::unique_ptr<CollisionShape> copy() const = 0;
  virtual std::unique_ptr<CollisionShape> transformed(const TransformChain& chain) const = 0;
  virtual ~CollisionShape() {}

#ifdef DEBUG
  virtual void draw(uint32_t *buffer, unsigned screen_h, unsigned screen_w) const = 0;
#endif
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
template <unsigned n_points>
Translation translate_(Point2d points[n_points], Point2d shift) {
  auto fwd = Translation(shift);
  for (Point2d *point = points; point != points+n_points; ++point) {
    *point = fwd(*point);
  }
  return Translation(-shift);
}
template <unsigned n_points>
Rotation rotate_(Point2d points[n_points], float rot) {
  auto fwd = Rotation(rot);
  for (Point2d *point = points; point != points+n_points; ++point) {
    *point = fwd(*point);
  }
  return Rotation(-rot);
}
template <unsigned n_points>
TranslationF translatef_(Point2d points[n_points], Point2d shift) {
  auto fwd = Translation(shift);
  for (Point2d *point = points; point != points+n_points; ++point) {
    *point = fwd(*point);
  }
  return TranslationF(Point2dF{.y=(float)-shift.y, .x=(float)-shift.x});
}
template <unsigned n_points>
RotationF rotatef_(Point2d points[n_points], float rot) {
  auto fwd = Rotation(rot);
  for (Point2d *point = points; point != points+n_points; ++point) {
    *point = fwd(*point);
  }
  return RotationF(-rot);
}

#ifdef DEBUG
void drawLine(Point2d from, Point2d to, uint32_t *buffer, unsigned screen_h, unsigned screen_w);
void drawCircle(Point2d center, float radius, uint32_t *buffer, unsigned screen_h, unsigned screen_w, float eps=0.1);
template <unsigned n_points>
void draw(const Point2d points[n_points], uint32_t *buffer, unsigned screen_h, unsigned screen_w) {
  for (unsigned i = 0; i != n_points; ++i) {
    drawLine(points[i], points[(i+1)%n_points], buffer, screen_h, screen_w);
  }
}
#endif // DEBUG
} // collision_shape

// correct points layout is not ensured
struct Triangle;
struct Sphere;
struct Rectangle : CollisionShape {
private:
  static constexpr Type type = Type::rectangle;
public:
  Point2d points[4];

  Rectangle() = default;
  Rectangle(Box2d);

  // returns inverse
  Translation translate_(Point2d shift);
  TranslationF translatef_(Point2d shift);
  // returns inverse
  Rotation rotate_(float rot);
  RotationF rotatef_(float rot);

  bool contains(Point2d point) const;

  bool collides(const CollisionShape& other, Type=Type::unknown) const override;
  bool collides(const Rectangle& other) const;
  bool collides(const Triangle& triangle) const;
  bool collides(const Sphere& sphere) const;

  std::unique_ptr<CollisionShape> transformed(const TransformChain& chain) const override;
  std::unique_ptr<CollisionShape> copy() const override { return transformed(TransformChain()); }

#ifdef DEBUG
  void draw(uint32_t *buffer, unsigned screen_h, unsigned screen_w) const override {
    collision_shape::draw<4>(points, buffer, screen_h, screen_w);
  }
#endif
};

struct Triangle : CollisionShape {
private:
  static constexpr Type type = Type::triangle;
public:
  Point2d points[3]; // counter-clockwise

  Triangle() = default;
  Triangle(Point2d f, Point2d s, Point2d t) { points[0]=f; points[1]=s; points[2]=t; }

  bool contains(Point2d point) const;

  bool collides(const CollisionShape& other, Type=Type::unknown) const override;
  bool collides(const Rectangle& rectangle) const;
  bool collides(const Triangle& other) const;
  bool collides(const Sphere& sphere) const;

  std::unique_ptr<CollisionShape> transformed(const TransformChain& chain) const override;
  std::unique_ptr<CollisionShape> copy() const override { return transformed(TransformChain()); }

#ifdef DEBUG
  void draw(uint32_t *buffer, unsigned screen_h, unsigned screen_w) const override {
    collision_shape::draw<3>(points, buffer, screen_h, screen_w);
  }
#endif
};

struct Sphere : CollisionShape {
  using spherical = void*;
private:
  static constexpr Type type = Type::sphere;
public:
  Point2d center;
  float radius;

  Sphere() = default;
  Sphere(Point2d c, float r): center(c), radius(r) {}

  bool contains(Point2d point) const;
  
  // bool collides(const Rectangle& rectangle) const;
  // bool collides(const Triangle& triangle) const;
  bool collides(const CollisionShape& other, Type=Type::unknown) const override;
  bool collides(const Sphere& other) const;
  template <class T>
  bool collides(const T& shape) const;

  std::unique_ptr<CollisionShape> transformed(const TransformChain& chain) const override;
  std::unique_ptr<CollisionShape> copy() const override { return transformed(TransformChain()); }

#ifdef DEBUG
  void draw(uint32_t *buffer, unsigned screen_h, unsigned screen_w) const override {
    collision_shape::drawCircle(center, radius, buffer, screen_h, screen_w);
  }
#endif
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

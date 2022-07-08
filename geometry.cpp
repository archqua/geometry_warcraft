#include "geometry.h"

#include <fstream>
#include <string>
#include <cmath>

float Point2d::angle(Point2d other) const {
  float normalizer = 1.0/(this->length() * other.length());
  float c = this->dot(other) * normalizer;
  float s = this->rot90().dot(other) * normalizer;
  float ang_mag = acos(c);
  return (1 - 2*(s > 0)) * ang_mag;
}

// obscure
Box2d boundingBox(const Box2d& square, float rot)
{
  float double_sin_half = 2 * sin(rot/2);
  float just_sin = sin(rot);
  float just_cos = cos(rot);
  unsigned width = abs(square.rb.x - square.lt.x);
  unsigned height = abs(square.rb.y - square.lt.y);
  // TODO round before cast??????
  Point2d rt{
    .y = (int)(square.lt.y - width*double_sin_half*just_cos),
    .x = (int)(square.lt.x - width*double_sin_half*just_sin),
  };
  Point2d lb{
    .y = (int)(square.rb.y - height*double_sin_half*just_sin),
    .x = (int)(square.lt.x + height*double_sin_half*just_cos),
  };

  Point2d diag_diff = square.rb - square.lt;
  diag_diff.y = (-diag_diff.x*just_sin) + diag_diff.y*just_cos;
  diag_diff.x = diag_diff.x*just_cos + diag_diff.y*just_sin;
  Point2d rb = square.lt + diag_diff;

  Point2d points[4] = {square.lt, rt, lb, rb};
  return boundingBox2d<4>(points);
}

Point2d operator*(float scalar, Point2d point)
{
  return point*scalar;
}

Box2d boundingBox2d(const Rectangle& rect)
{
  return boundingBox2d<4>(rect.points);
}

Rectangle::Rectangle(Box2d box) {
  points[0] = box.lt;
  points[1] = Point2d{.y = box.rb.y, .x = box.lt.x};
  points[2] = box.rb;
  points[3] = Point2d{.y = box.lt.y, .x = box.rb.x};
}

bool Rectangle::contains(Point2d point) const {
  for (unsigned i = 0; i < 4; ++i) {
    if ((point - points[i]).dot(points[(i+1)%4] - points[i]) < 0) {
      return false;
    }
  }
  return true;
}

bool Triangle::contains(Point2d point) const {
  // points are counter-clockwise
  for (unsigned i = 0; i < 3; ++i) {
    Point2d diff90 = (points[(i+1)%3] - points[i]).rot90();
    if (diff90.dot(point - points[i]) < 0) {
      return false;
    }
  }
  return true;
}

bool Sphere::contains(Point2d point) const {
  return (point - center).length() < radius;
}

Translation Rectangle::translate_(Point2d shift) {
  // auto trans = Translation(shift);
  // for (Point2d *point = points; point < points+4; ++point) {
  //   *point = trans(*point);
  // }
  // return Translation(-shift);
  return collision_shape::translate_<4>(points, shift);
}

Rotation Rectangle::rotate_(float rot) {
  // auto rota = Rotation(rot);
  // for (Point2d *point = points; point < points+4; ++point) {
  //   *point = rota(*point);
  // }
  // return Rotation(-rot);
  return collision_shape::rotate_<4>(points, rot);
}

void TransformChain::append(std::unique_ptr<Transform> transform) {
  transforms.push_back(std::move(transform));
}
Point2d  TransformChain::backward(Point2d point) const {
  for (auto biter = transforms.rbegin(); biter != transforms.rend(); ++biter) {
    point = (**biter)(point);
  }
  return point;
}
Point2d  TransformChain::forward(Point2d point) const {
  for (auto biter = transforms.begin(); biter != transforms.end(); ++biter) {
    point = (**biter)(point);
  }
  return point;
}

Point2d Translation::operator()(Point2d point) const {
  return point + shift;
}

Point2d Rotation::operator()(Point2d point) const {
  // TODO rounding instead of (int)???
  return Point2d{
    // rounding causes segfault
    // .y = (int)roundf(-point.x*s + point.y*c),
    // .x = (int)roundf(point.x*c + point.y*s),
    .y = (int)(-point.x*s + point.y*c),
    .x = (int)(point.x*c + point.y*s),
  };
}

// box might break
void Box2d::intersect_(const Box2d& other) {
  if (lt.y < other.lt.y) {
    lt.y = other.lt.y;
  }
  if (lt.x < other.lt.x) {
    lt.x = other.lt.x;
  }

  if (rb.y > other.rb.y) {
    rb.y = other.rb.y;
  }
  if (rb.x > other.rb.x) {
    rb.x = other.rb.x;
  }
}

bool Box2d::contains(Point2d point) const {
  return lt.y <= point.y && lt.x <= point.x && rb.y > point.y && rb.x > point.x;
}

bool Rectangle::collides(const Rectangle& other) const {
  return ::collides<4, 4>(points, other.points);
}

bool Rectangle::collides(const Triangle& triangle) const {
  return ::collides<4, 3>(points, triangle.points);
}

bool Rectangle::collides(const Sphere& sphere) const {
  return sphere.collides(*this);
}

bool Triangle::collides(const Rectangle& rectangle) const {
  return rectangle.collides(*this);
}

bool Triangle::collides(const Triangle& other) const {
  return ::collides<3, 3>(points, other.points);
}

bool Triangle::collides(const Sphere& sphere) const {
  return sphere.collides(*this);
}

// bool Sphere::collides(const Rectangle& rectangle) const {
//   return collides<Rectangle>(rectangle);
// }

// bool Sphere::collides(const Triangle& triangle) const {
//   return collides<Triangle>(triangle);
// }

bool Sphere::collides(const Sphere& other) const {
  return (center - other.center).length() > (radius + other.radius);
}

[[noreturn]] void UNREACHABLE() {
  throw "unreachable";
}

bool Rectangle::collides(const CollisionShape& other, Type _type) const {
  switch(_type) {
    case Type::unknown:
      return other.collides(*this, this->type);
    case Type::rectangle:
      return collides(static_cast<const Rectangle&>(other));
    case Type::triangle:
      return collides(static_cast<const Triangle&>(other));
    case Type::sphere:
      return collides(static_cast<const Sphere&>(other));
  }
  UNREACHABLE();
}

bool Triangle::collides(const CollisionShape& other, Type _type) const {
  switch(_type) {
    case Type::unknown:
      return other.collides(*this, this->type);
    case Type::rectangle:
      return collides(static_cast<const Rectangle&>(other));
    case Type::triangle:
      return collides(static_cast<const Triangle&>(other));
    case Type::sphere:
      return collides(static_cast<const Sphere&>(other));
  }
  UNREACHABLE();
}

bool Sphere::collides(const CollisionShape& other, Type _type) const {
  switch(_type) {
    case Type::unknown:
      return other.collides(*this, this->type);
    case Type::rectangle:
      return collides(static_cast<const Rectangle&>(other));
    case Type::triangle:
      return collides(static_cast<const Triangle&>(other));
    case Type::sphere:
      return collides(static_cast<const Sphere&>(other));
  }
  UNREACHABLE();
}

std::unique_ptr<CollisionShape> collision_shape::readFromStream(std::istream& is)
{
  std::string type;
  if (getline(is, type)) {
    if ("triangle" == type) {
      std::unique_ptr<Triangle> triangle;
      for (Point2d *point = triangle->points; point != triangle->points+3; ++point) {
        is >> point->y >> point->x;
      }
      return triangle;
    } else if ("rectangle" == type) {
      std::unique_ptr<Rectangle> rectangle;
      for (Point2d *point = rectangle->points; point != rectangle->points+3; ++point) {
        is >> point->y >> point->x;
      }
      return rectangle;
    } else if ("sphere" == type) {
      std::unique_ptr<Sphere> sphere;
      is >> sphere->center.y >> sphere->center.x >> sphere->radius;
      return sphere;
    } else {
      throw SRError(type.data());
    }
  } else {
    throw SRError(type.data());
  }
}

std::unique_ptr<CollisionShape> Rectangle::transformed(const TransformChain& chain) const {
  auto res = std::make_unique<Rectangle>();
  const Point2d *src = points;
  Point2d *dst = res->points;
  while (src != points+4) {
    *dst++ = chain.forward(*src++);
  }
  return res;
}

std::unique_ptr<CollisionShape> Triangle::transformed(const TransformChain& chain) const {
  auto res = std::make_unique<Triangle>();
  const Point2d *src = points;
  Point2d *dst = res->points;
  while (src != points+3) {
    *dst++ = chain.forward(*src++);
  }
  return res;
}

std::unique_ptr<CollisionShape> Sphere::transformed(const TransformChain& chain) const {
  auto res = std::make_unique<Sphere>();
  res->radius = radius;
  res->center = chain.forward(center);
  return res;
}

#ifdef DEBUG
uint32_t red = 0 ^ (255 << 24) ^ (255 << 16);
void collision_shape::drawLine(Point2d from, Point2d to, uint32_t *buffer, unsigned screen_h, unsigned screen_w)
{
  int tilt = (float)(to.y - from.y) / (float)(to.x - from.x);
  Point2d arr[2] = {from, to};
  Box2d bounding_box = boundingBox2d<2>(arr);
  Box2d screen_box = Box2d{
    .lt = Point2d{.y = 0, .x = 0},
    .rb = Point2d{.y = (int)screen_h, .x = (int)screen_w},
  };
  bounding_box.intersect_(screen_box);
  // this is super inefficient but who cares
  for (int i = from.y; i != to.y; i += 1 - 2*((to.y - from.y) < 0)) {
    int j = from.x + (float)(i - from.y) / tilt;
      if (bounding_box.contains(Point2d{.y=i, .x=j,})) {
        buffer[i*screen_w + j] = red;
      }
  }
}

void collision_shape::drawCircle(Point2d center, float radius, uint32_t *buffer, unsigned screen_h, unsigned screen_w, float eps) {
  Box2d bounding_box = Box2d{
    .lt = Point2d{.y = (int)(center.y - radius), .x = (int)(center.x - radius),},
    .rb = Point2d{.y = (int)(center.y + radius), .x = (int)(center.x + radius),},
  };
  Box2d screen_box = Box2d{
    .lt = Point2d{.y = 0, .x = 0},
    .rb = Point2d{.y = (int)screen_h, .x = (int)screen_w},
  };
  bounding_box.intersect_(screen_box);
  for (int i = bounding_box.lt.y; i < bounding_box.rb.y; ++i) {
    for (int j = bounding_box.lt.x; j < bounding_box.rb.x; ++j) {
      float dist2center = ((Point2d{.y=i, .x=j} - center).length()/radius);
      if (dist2center < 1 && dist2center > 1-eps) {
        buffer[i*screen_w + j] = red;
      }
    }
  }
}

#endif // DEBUG

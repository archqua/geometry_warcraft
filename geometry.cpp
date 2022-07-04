#include "geometry.h"

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

template <unsigned n_points>
Box2d boundingBox2d(const Point2d points[n_points]) {
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

Translation Rectangle::translate_(Point2d shift) {
  auto trans = Translation(shift);
  for (Point2d *point = points; point < points+4; ++point) {
    *point = trans(*point);
  }
  return Translation(-shift);
}

Rotation Rectangle::rotate_(float rot) {
  auto rota = Rotation(rot);
  for (Point2d *point = points; point < points+4; ++point) {
    *point = rota(*point);
  }
  return Rotation(-rot);
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

Point2d Translation::operator()(Point2d point) const {
  return point + shift;
}

Point2d Rotation::operator()(Point2d point) const {
  // TODO rounding instead of (int)???
  return Point2d{
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

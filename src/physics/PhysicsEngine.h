#ifndef PHYSICSENGINE_H
#define PHYSICSENGINE_H
#include <memory>
#include <vector>

#include "../AABB.h"
#include "../Circle.h"
#include "../Polygon.h"
#include "../Scene.h"
#include "Manifold.h"

class PhysicsEngine {
public:
  static bool checkCollisionCircleCircle(const Circle &circle1,
                                         const Circle &circle2);
  static void resolveCollisionCircleCircle(Circle &circle1, Circle &circle2);
  static bool checkCollisionAABBCircle(const AABB &aabb, const Circle &circle);
  static void resolveCollisionAABBCircle(Manifold &m, AABB &aabb, Circle &circle);

  static void positionalCorrection(Circle &circle1, Circle &circle2);
  static bool checkCollisionPolygonPolygon(const Polygon &p1,
                                           const Polygon &p2);
  static bool checkCollisionPolygonAABB(const Polygon &p, const AABB &aabb);
  static void resolveCollisionPolygonAABB(Manifold &m, Polygon &polygon);

};

#endif

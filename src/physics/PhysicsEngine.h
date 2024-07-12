#ifndef PHYSICSENGINE_H
#define PHYSICSENGINE_H
#include <memory>
#include <vector>

#include "Manifold.h"
#include "../AABB.h"
#include "../Circle.h"
#include "../Polygon.h"

class PhysicsEngine {
public:
    static void update(std::vector<Object*>* objects, float deltaTime);
    static bool checkCollisionCircleCircle(const Circle &circle1, const Circle &circle2);
    static void resolveCollisionCircleCircle(Circle &circle1, Circle &circle2);
    static bool checkCollisionAABBCircle(const AABB &aabb, const Circle &circle);
    static void resolveCollisionAABBCircle(Manifold &m);
    static void positionalCorrection(Circle &circle1, Circle &circle2);
    static bool checkCollisionPolygonPolygon(const Polygon &p1, const Polygon &p2);
    static bool checkCollisionPolygonAABB(const Polygon &p, const AABB &aabb);
};

#endif

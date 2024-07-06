#ifndef PHYSICSENGINE_H
#define PHYSICSENGINE_H
#include <memory>
#include <vector>

#include "../AABB.h"
#include "../Circle.h"

class PhysicsEngine {
public:
    static void update(std::vector<Object*>* objects, float deltaTime);
    static bool checkCollisionCircleCircle(const Circle &circle1, const Circle &circle2);
    static void resolveCollisionCircleCircle(Circle &circle1, Circle &circle2);
    static bool checkCollisionAABBCircle(const AABB &aabb, const Circle &circle);
    static void resolveCollisionAABBCircle(AABB &aabb, Circle &circle);
};

#endif
#ifndef MANIFOLD_H
#define MANIFOLD_H
#include "../AABB.h"
#include "../Circle.h"
#include "../Object.h"
#include "../Polygon.h"
#include "glm/vec3.hpp"

struct Manifold {
    Object *A;
    Object *B;
    float penetration;
    glm::vec3 normal;

    bool CirclevsCircle(Circle &circle1, Circle &circle2);
    bool AABBvsCircle(AABB &aabb, Circle &circle);
    bool PolygonvsAABB(Polygon &polygon, AABB &aabb);
};

#endif

#ifndef MANIFOLD_H
#define MANIFOLD_H
#include "../AABB.h"
#include "../Box.h"
#include "../Circle.h"
#include "../Object.h"
#include "../Polygon.h"
#include "glm/vec3.hpp"

struct Manifold {
    glm::vec3 normal;
    float penetration;
    glm::vec3 contactPoint1;
    glm::vec3 contactPoint2;
    int nContacts;

    bool CirclevsCircle(Circle &circle1, Circle &circle2);
    bool CirclevsCircle(const glm::vec3 &centerA, float radiusA, const glm::vec3 &centerB, float radiusB);
    bool AABBvsCircle(AABB &aabb, Circle &circle);
    bool PolygonvsAABB(Polygon &polygon, AABB &aabb);
    bool CirclevsBox(const glm::vec3 &circleCenter, float circleRadius, const std::vector<glm::vec3> &boxVertices, const glm::vec3 &boxCenter);
    bool BoxvsBox(const std::vector<glm::vec3> &boxVerticesA, const glm::vec3 &boxCenterA, const std::vector<glm::vec3> &boxVerticesB, const glm::vec3 &boxCenterB);

};

#endif

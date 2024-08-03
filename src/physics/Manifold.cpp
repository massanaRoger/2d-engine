//
// Created by Roger2 on 08/07/2024.
//

#include "Manifold.h"
#include "../Polygon.h"
#include "../utils.h"
#include <limits>
#define GLM_ENABLE_EXPERIMENTAL

#include "contacts.h"
#include "glm/gtx/norm.hpp"

bool Manifold::PolygonvsAABB(Polygon &polygon, AABB &aabb) {

    // Get the vertices of the AABB
    std::vector aabbVertices = {
        glm::vec3(aabb.aabbc->min.x, aabb.aabbc->min.y, 0.0f),
        glm::vec3(aabb.aabbc->max.x, aabb.aabbc->min.y, 0.0f),
        glm::vec3(aabb.aabbc->max.x, aabb.aabbc->max.y, 0.0f),
        glm::vec3(aabb.aabbc->min.x, aabb.aabbc->max.y, 0.0f)
    };

    std::vector<glm::vec3> normals = calculateNormals(polygon.pc->vertices);
    normals.emplace_back(1.0f, 0.0f, 0.0f); // AABB's normal (x-axis)
    normals.emplace_back(0.0f, 1.0f, 0.0f); // AABB's normal (y-axis)
    
    float minPenetration = std::numeric_limits<float>::max();
    glm::vec3 collisionNormal;

    for (const auto& vertexNormal : normals) {
        float min1, max1, min2, max2;
        projectPolygon(polygon.pc->vertices, vertexNormal, min1, max1);
        projectPolygon(aabbVertices, vertexNormal, min2, max2);
        float penetrationDepth = std::min(max1, max2) - std::max(min1, min2);
        if (penetrationDepth < minPenetration) {
            minPenetration = penetrationDepth;
            collisionNormal = vertexNormal;
        }
    }

    normal = collisionNormal;
    penetration = minPenetration;
    return true;
}

bool Manifold::CirclevsCircle(Circle &circle1, Circle &circle2) {
    // Vector from A to B
    glm::vec3 n = circle2.pc->position - circle1.pc->position;
    float r = circle1.cc->radius + circle2.cc->radius;
    r *= r;
    if(glm::length2(n) > r)
        return false;
    // Circles have collided, now compute manifold
    float d = glm::length(n); // perform actual sqrt
    // If distance between circles is not zero
    if(d != 0)
    {
        // Distance is difference between radius and distance
        penetration = r - d;
        // Utilize our d since we performed sqrt on it already within Length( )
        // Points from A to B, and is a unit vector
        normal = n / d;
        return true;
    }
        // Circles are on same position
    else
    {
        // Choose random (but consistent) values
        penetration = circle1.cc->radius;
        normal = glm::vec3(1.0f, 0.0f, 0.0f);
        return true;
    }
}

bool Manifold::AABBvsCircle(AABB &aabb, Circle &circle) {
    glm::vec3 aabbCenter = glm::vec3((aabb.aabbc->min + aabb.aabbc->max) / 2.0f);
    glm::vec3 n = aabbCenter - circle.pc->position;

    // Closest point on A to center of B
    glm::vec3 closest = n;
    // Calculate half extents along each axis
    float x_extent = (aabb.aabbc->max.x - aabb.aabbc->min.x) / 2;
    float y_extent = (aabb.aabbc->max.y - aabb.aabbc->min.y) / 2;
    // Clamp point to edges of the AABB
    closest.x = glm::clamp(-x_extent, x_extent, closest.x);
    closest.y = glm::clamp(-y_extent,y_extent, closest.y);
    bool inside = false;
    // Circle is inside the AABB, so we need to clamp the circle's center
    // to the closest edge
    if(n == closest)
    {
        inside = true;
        // Find closest axis
        if(abs( n.x ) > abs( n.y ))
        {
            // Clamp to closest extent
            if(closest.x > 0)
                closest.x = x_extent;
              else
                  closest.x = -x_extent;
              }
        // y axis is shorter
        else
        {
            // Clamp to closest extent
            if(closest.y > 0)
                closest.y = y_extent;
              else
                  closest.y = -y_extent;
              }
    }
    glm::vec3 tempNorm = n - closest;
    float d = glm::length2(tempNorm);
    float r = circle.cc->radius;
    // Early out of the radius is shorter than distance to closest point and
    // Circle not inside the AABB
    if(d > r * r && !inside)
        return false;
      // Avoided sqrt until we needed
      d = glm::length(tempNorm);
      // Collision normal needs to be flipped to point outside if circle was
      // inside the AABB
      if(inside)
      {
          normal = -n;
          penetration = r - d;
        }
      else
      {
          normal = n;
          penetration = r - d;
        }
    return true;
}

bool Manifold::CirclevsBox(const glm::vec3 &circleCenter, float circleRadius, const std::vector<glm::vec3> &boxVertices, const glm::vec3 &boxCenter) {
    normal = glm::vec3{};
    penetration = std::numeric_limits<float>::max();
    float minA, maxA;
    float minB, maxB;

    for (int i = 0; i < boxVertices.size(); i++) {
        glm::vec3 va = boxVertices[i];
        glm::vec3 vb = boxVertices[(i + 1) % boxVertices.size()];

        glm::vec3 edge = vb - va;
        glm::vec3 axis = glm::vec3(-edge.y, edge.x, 0.0f);


        projectPolygon(boxVertices, axis, minA, maxA);
        projectCircle(circleCenter, circleRadius, axis, minB, maxB);

        if (minA >= maxB || minB >= maxA) {
            return false;
        }

        float axisDepth = std::min(maxB - minA, maxA - minB);

        if (axisDepth < penetration) {
            penetration = axisDepth;
            normal = axis;
        }
    }

    int cpIndex = findClosestPointOnPolygon(circleCenter, boxVertices);
    glm::vec3 cp = boxVertices[cpIndex];

    glm::vec3 axis = cp - circleCenter;

    projectPolygon(boxVertices, axis, minA, maxA);
    projectCircle(circleCenter, circleRadius, axis, minB, maxB);

    if (minA >= maxB || minB >= maxA) {
        return false;
    }

    float axisDepth = std::min(maxB - minA, maxA - minB);

    if (axisDepth < penetration) {
        penetration = axisDepth;
        normal = axis;
    }

    penetration /= glm::length(normal);
    normal = glm::normalize(normal);

    glm::vec3 direction = boxCenter - circleCenter;

    if (glm::dot(direction, normal) < 0.0f) {
        normal = -normal;
    }

    contactPoint1 = contactPointCirclevsBox(circleCenter, boxVertices);
    nContacts = 1;

    return true;
}


//
// Created by Roger2 on 08/07/2024.
//

#include "Manifold.h"
#define GLM_ENABLE_EXPERIMENTAL
#include "glm/gtx/norm.hpp"

bool Manifold::CirclevsCircle(Circle &circle1, Circle &circle2) {
    A = &circle1;
    B = &circle2;
    // Vector from A to B
    glm::vec3 n = circle2.position - circle1.position;
    float r = circle1.radius + circle2.radius;
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
        penetration = circle1.radius;
        normal = glm::vec3(1.0f, 0.0f, 0.0f);
        return true;
    }
}

bool Manifold::AABBvsCircle(AABB &aabb, Circle &circle) {
    A = &aabb;
    B = &circle;
    glm::vec3 aabbCenter = glm::vec3((aabb.min + aabb.max) / 2.0f, 0.0f);
    glm::vec3 n = aabbCenter - circle.position;

    // Closest point on A to center of B
    glm::vec3 closest = n;
    // Calculate half extents along each axis
    float x_extent = (aabb.max.x - aabb.min.x) / 2;
    float y_extent = (aabb.max.y - aabb.min.y) / 2;
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
    float r = circle.radius;
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

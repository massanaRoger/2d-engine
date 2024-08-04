#include "PhysicsEngine.h"

#include "../Polygon.h"
#include "../components/Components.h"
#include "Manifold.h"

#include "../utils.h"
#include <cmath>

void PhysicsEngine::resolveCollisionPolygonAABB(Manifold &m, Polygon &polygon) {

  glm::vec3 collisionNormal = m.normal;
  float penetration = m.penetration;

  // Calculate relative velocity
  glm::vec3 relativeVelocity = polygon.vc->velocity;

  float velocityAlongNormal = glm::dot(relativeVelocity, collisionNormal);
  if (velocityAlongNormal > 0) {
    return;
  }

  // Calculate restitution
  float e = 1.0f; // Assume elasticity (restitution)
  float j = -(1 + e) * velocityAlongNormal;
  j /= polygon.mc->inverseMass;

  glm::vec3 impulse = collisionNormal * j;
  polygon.vc->velocity += impulse * polygon.mc->inverseMass;

  // Calculate the point of collision (assuming center of mass for simplicity)
  glm::vec3 r = collisionNormal * penetration;
  glm::vec3 angularImpulse = glm::cross(r, impulse);

  // Apply angular impulse
  // polygon.avc->angularVelocity += angularImpulse.z * polygon.ic->inverseInertia;

  // Positional correction to avoid sinking
  const float percent = 0.8f; // usually 20% to 80%
  const float slop = 0.01f;   // usually 0.01 to 0.1
  glm::vec3 correction = std::max(penetration - slop, 0.0f) /
                         polygon.mc->inverseMass * percent * collisionNormal;

  for (auto &v : polygon.pc->vertices) {
    v += correction;
  }
}

bool PhysicsEngine::checkCollisionCircleCircle(const Circle &circle1,
                                               const Circle &circle2) {
  float r = circle1.cc->radius + circle2.cc->radius;
  r *= r;
  float dx = circle1.pc->position.x - circle2.pc->position.x;
  float dy = circle1.pc->position.y - circle2.pc->position.y;
  float circleDistance = dx * dx + dy * dy;
  return circleDistance < r;
}

void PhysicsEngine::resolveCollisionCircleCircle(Circle &circle1,
                                                 Circle &circle2) {
  glm::vec3 collisionNormal = circle2.pc->position - circle1.pc->position;
  collisionNormal = glm::normalize(collisionNormal);
  glm::vec3 relativeVelocity = circle2.vc->velocity - circle1.vc->velocity;

  float velocityAlongNormal = glm::dot(relativeVelocity, collisionNormal);
  if (velocityAlongNormal > 0) {
    return;
  }

  // Calculate restitution
  float e = 1.0f; // You might want to make this a property of the circles if
                  // different circles have different restitution values
  float j = -(1 + e) * velocityAlongNormal;
  j /= (circle1.mc->inverseMass + circle2.mc->inverseMass);

  glm::vec3 impulse = collisionNormal * j;
  circle1.vc->velocity = circle1.vc->velocity - impulse * circle1.mc->inverseMass;
  circle2.vc->velocity = circle2.vc->velocity + impulse * circle2.mc->inverseMass;
}

#if false
bool PhysicsEngine::checkCollisionAABBCircle(const AABB &aabb,
                                             const Circle &circle) {
  const float closestX =
      std::max(aabb.aabbc->min.x, std::min(circle.pc->position.x, aabb.aabbc->max.x));
  const float closestY =
      std::max(aabb.aabbc->min.y, std::min(circle.pc->position.y, aabb.aabbc->max.y));

  const float distanceX = circle.pc->position.x - closestX;
  const float distanceY = circle.pc->position.y - closestY;

  const float distanceSquared =
      (distanceX * distanceX) + (distanceY * distanceY);
  return distanceSquared < (circle.cc->radius * circle.cc->radius);
}
#endif

void PhysicsEngine::resolveCollisionAABBCircle(const std::vector<glm::vec3> &boxVertices, glm::vec3 &circleCenter, glm::vec3 &circleVelocity, float circleInverseMass, float circleRadius) {
  glm::vec3 closestPoint;

  glm::vec3 min = boxVertices[0];
  glm::vec3 max = boxVertices[2];
  closestPoint.x =
      std::max(min.x, std::min(circleCenter.x, max.x));
  closestPoint.y =
      std::max(min.y, std::min(circleCenter.y, max.y));
  closestPoint.z = 0;

  glm::vec3 collisionNormal = circleCenter - closestPoint;
  collisionNormal = glm::normalize(collisionNormal);

  glm::vec3 relativeVelocity = circleVelocity;

  float velocityAlongNormal = glm::dot(relativeVelocity, collisionNormal);
  if (velocityAlongNormal > 0) {
    return;
  }

  // Calculate restitution
  float e = 1.0f;
  float j = -(1 + e) * velocityAlongNormal;
  j /= circleInverseMass;

  glm::vec3 impulse = collisionNormal * j;
  circleVelocity = circleVelocity + impulse * circleInverseMass;

  // Ensure the circle is pushed out of the AABB
  float penetrationDepth =
      circleRadius - glm::length(circleCenter - closestPoint);
  if (penetrationDepth > 0) {
    circleCenter += collisionNormal * penetrationDepth;
  }
}

bool PhysicsEngine::checkCollisionPolygonPolygon(const Polygon &p1,
                                                 const Polygon &p2) {
  std::vector<glm::vec3> p1Vertices = p1.pc->vertices;
  std::vector<glm::vec3> p2Vertices = p2.pc->vertices;
  std::vector<glm::vec3> normals1 = calculateNormals(p1Vertices);
  std::vector<glm::vec3> normals2 = calculateNormals(p2Vertices);

  for (auto &normal : normals1) {
    if (!overlapOnAxis(p1Vertices, p2Vertices, normal)) {
      return false;
    }
  }

  for (auto &normal : normals2) {
    if (!overlapOnAxis(p1Vertices, p2Vertices, normal)) {
      return false;
    }
  }
  return true;
}

bool PhysicsEngine::checkCollisionBoxBox(const Box &box1, const Box &box2) {
  std::vector<glm::vec3> vertices1 = calculateAABBvertices(*(box1.min), *(box1.max));
  std::vector<glm::vec3> normals1 = calculateNormals(vertices1);

  std::vector<glm::vec3> vertices2 =
      calculateAABBvertices(*(box2.min), *(box2.max));
  std::vector<glm::vec3> normals2 = calculateNormals(vertices2);

  for (auto &normal : normals1) {
    if (!overlapOnAxis(vertices1, vertices2, normal)) {
      return false;
    }
  }

  for (auto &normal : normals2) {
    if (!overlapOnAxis(vertices1, vertices2, normal)) {
      return false;
    }
  }
  return true;
}

void PhysicsEngine::resolveCollisionBoxBox(const Box &box1, const Box &box2) {

}

float PhysicsEngine::calculateMomentOfInertia(const glm::vec3 &min, const glm::vec3 &max, float mass) {
  float width = max.x - min.x;
  float height = max.y - min.y;

  float momentOfInertia = (mass / 12.0f) * (width * width + height * height);

  return momentOfInertia;
}


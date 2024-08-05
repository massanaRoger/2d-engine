#include "PhysicsEngine.h"

#include "../Polygon.h"
#include "../components/Components.h"
#include "Manifold.h"

#include "../utils.h"
#include <cmath>

#include "Transformations.h"

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

void PhysicsEngine::resolveRotationalCollision(const Manifold &m, const glm::vec3 &boxCenter1,
  glm::vec3 &boxLinearVelocity1, float &boxAngularVelocity1, float boxInvInertia1, float boxInvMass1,
  glm::vec3 &boxCenter2, glm::vec3 &boxLinearVelocity2, float &boxAngularVelocity2, float boxInverseMass2, float boxInverseInertia2) {

  // Todo: calculate minimum restitution between the box and circle, for the moment we assume arbitrary value
  float e = 0.7f;

  std::vector<glm::vec3> contactList{};
  assert(m.nContacts == 1 || m.nContacts == 2);
  contactList.push_back(m.contactPoint1);
  if (m.nContacts == 2) {
    contactList.push_back(m.contactPoint2);
  }

  glm::vec3 impulseList[2];
  glm::vec3 raList[2];
  glm::vec3 rbList[2];
  bool ignoreContact[2] = {false, false};

  for (int i = 0; i < m.nContacts; i++) {
    glm::vec3 ra = contactList[i] - boxCenter1;
    glm::vec3 rb = contactList[i] - boxCenter2;

    raList[i] = ra;
    rbList[i] = rb;

    glm::vec3 raPerp(ra.y, -ra.x, 0.0f);
    glm::vec3 rbPerp(rb.y, -rb.x, 0.0f);

    glm::vec3 angularLinearVelocityA = raPerp * boxAngularVelocity1;
    glm::vec3 angularLinearVelocityB = rbPerp * boxAngularVelocity2;

    glm::vec3 relativeVelocity = (boxLinearVelocity2 + angularLinearVelocityB) - (boxLinearVelocity1 + angularLinearVelocityA);

    float contactVelocityMagnitude = glm::dot(relativeVelocity, m.normal);

    if (contactVelocityMagnitude > 0.0f) {
      ignoreContact[i] = true;
      continue;
    }

    float raPerpDotN = glm::dot(raPerp, m.normal);
    float rbPerpDotN = glm::dot(rbPerp, m.normal);

    float denom = boxInvMass1 + boxInverseMass2 +
      (raPerpDotN * raPerpDotN) * boxInvInertia1 +
      (rbPerpDotN * rbPerpDotN) * boxInverseInertia2;

    float j = -(1.0f + e) * contactVelocityMagnitude;
    j /= denom;

    j /= static_cast<float>(m.nContacts);

    glm::vec3 impulse = j * m.normal;
    impulseList[i] = impulse;
  }

  for (int i = 0; i < m.nContacts; i++) {
    if (ignoreContact[i]) continue;
    glm::vec3 impulse = impulseList[i];

    boxLinearVelocity1 += -impulse * boxInvMass1;
    boxAngularVelocity1 += -Transformations::cross(glm::vec2(raList[i].x, raList[i].y), glm::vec2(impulse.x, impulse.y)) * boxInvInertia1;
    boxLinearVelocity2 += impulse * boxInverseMass2;
    boxAngularVelocity2 += Transformations::cross(glm::vec2(rbList[i].x, rbList[i].y), glm::vec2(impulse.x, impulse.y)) * boxInverseInertia2;
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


#ifndef PHYSICSENGINE_H
#define PHYSICSENGINE_H
#include <memory>
#include <vector>

#include "Manifold.h"

class PhysicsEngine {
public:
  static void resolveRotationalCollision(const Manifold &m, const glm::vec3 &boxCenter,
      glm::vec3 &boxLinearVelocity, float &boxAngularVelocity, float boxInvInertia, float boxInvMass,
      glm::vec3 &circleCenter, glm::vec3 &circleVelocity, float &circleAngularVelocity, float circleInverseMass, float circleInverseInertia);

  static float calculateMomentOfInertia(const glm::vec3 &min, const glm::vec3 &max, float mass);

  static bool checkCollisionCircleBox(const glm::vec3 &circleCenter, float circleRadius,const std::vector<glm::vec3> &boxVertices);

};

#endif

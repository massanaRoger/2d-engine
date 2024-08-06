#ifndef PHYSICSENGINE_H
#define PHYSICSENGINE_H
#include <memory>
#include <vector>

#include "Manifold.h"

class PhysicsEngine {
public:
    static void resolveRotationalCollision(const Manifold &m, const glm::vec3 &boxCenter1,
    glm::vec3 &boxLinearVelocity1, float &boxAngularVelocity1, float boxInvInertia1, float boxInvMass1,
    glm::vec3 &boxCenter2, glm::vec3 &boxLinearVelocity2, float &boxAngularVelocity2, float boxInverseMass2, float boxInverseInertia2);

    static void resolveRotationalCollisionWithFriction(const Manifold &m, const glm::vec3 &boxCenter1,
  glm::vec3 &boxLinearVelocity1, float &boxAngularVelocity1, float boxInvInertia1, float boxInvMass1,
  float staticFriction1, float dynamicFriction1, glm::vec3 &boxCenter2, glm::vec3 &boxLinearVelocity2,
  float &boxAngularVelocity2, float boxInverseMass2, float boxInverseInertia2, float staticFriction2, float dynamicFriction2);

  static float calculateMomentOfInertia(const glm::vec3 &min, const glm::vec3 &max, float mass);

  static bool checkCollisionCircleBox(const glm::vec3 &circleCenter, float circleRadius,const std::vector<glm::vec3> &boxVertices);

};

#endif

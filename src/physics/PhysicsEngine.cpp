#include "PhysicsEngine.h"

#include "../components/Components.h"
#include "Manifold.h"

#include "../utils.h"
#include <cmath>

#include "contacts.h"
#include "Transformations.h"

void PhysicsEngine::resolveRotationalCollision(const Manifold &m, const glm::vec3 &boxCenter1,
  glm::vec3 &boxLinearVelocity1, float &boxAngularVelocity1, float boxInvInertia1, float boxInvMass1,
  glm::vec3 &boxCenter2, glm::vec3 &boxLinearVelocity2, float &boxAngularVelocity2, float boxInverseMass2, float boxInverseInertia2) {

  // Todo: calculate minimum restitution between the box and circle, for the moment we assume arbitrary value
  float e = 0.8f;

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

    glm::vec3 raPerp(-ra.y, ra.x, 0.0f);
    glm::vec3 rbPerp(-rb.y, rb.x, 0.0f);

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

void PhysicsEngine::resolveRotationalCollisionWithFriction(const Manifold &m, const glm::vec3 &boxCenter1,
  glm::vec3 &boxLinearVelocity1, float &boxAngularVelocity1, float boxInvInertia1, float boxInvMass1,
  float staticFriction1, float dynamicFriction1, glm::vec3 &boxCenter2, glm::vec3 &boxLinearVelocity2,
  float &boxAngularVelocity2, float boxInverseMass2, float boxInverseInertia2, float staticFriction2, float dynamicFriction2) {

  // Todo: calculate minimum restitution between the box and circle, for the moment we assume arbitrary value
  float e = 0.8f;

  float sf = (staticFriction1 + staticFriction2) * 0.5f;
  float df = (dynamicFriction1 + dynamicFriction2) * 0.5f;

  std::vector<glm::vec3> contactList{};
  assert(m.nContacts == 1 || m.nContacts == 2);
  contactList.push_back(m.contactPoint1);
  if (m.nContacts == 2) {
    contactList.push_back(m.contactPoint2);
  }

  glm::vec3 impulseList[2] = {glm::vec3(0.0f), glm::vec3(0.0f)};
  glm::vec3 frictionImpulseList[2] = {glm::vec3(0.0f), glm::vec3(0.0f)};
  glm::vec3 raList[2] = {glm::vec3(0.0f), glm::vec3(0.0f)};
  glm::vec3 rbList[2] = {glm::vec3(0.0f), glm::vec3(0.0f)};
  float jList[2] = {0.0f, 0.0f};
  bool ignoreContact[2] = {false, false};

  for (int i = 0; i < m.nContacts; i++) {
    glm::vec3 ra = contactList[i] - boxCenter1;
    glm::vec3 rb = contactList[i] - boxCenter2;

    raList[i] = ra;
    rbList[i] = rb;

    glm::vec3 raPerp(-ra.y, ra.x, 0.0f);
    glm::vec3 rbPerp(-rb.y, rb.x, 0.0f);

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
    jList[i] = j;

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

  for (int i = 0; i < m.nContacts; i++) {
    glm::vec3 ra = contactList[i] - boxCenter1;
    glm::vec3 rb = contactList[i] - boxCenter2;

    raList[i] = ra;
    rbList[i] = rb;

    glm::vec3 raPerp(-ra.y, ra.x, 0.0f);
    glm::vec3 rbPerp(-rb.y, rb.x, 0.0f);

    glm::vec3 angularLinearVelocityA = raPerp * boxAngularVelocity1;
    glm::vec3 angularLinearVelocityB = rbPerp * boxAngularVelocity2;

    glm::vec3 relativeVelocity = (boxLinearVelocity2 + angularLinearVelocityB) - (boxLinearVelocity1 + angularLinearVelocityA);

    glm::vec3 tangent = relativeVelocity - glm::dot(relativeVelocity, m.normal) * m.normal;

    if (nearlyEqual(tangent, glm::vec3(0.0f))) {
      continue;
    }
    tangent = glm::normalize(tangent);

    float raPerpDotT = glm::dot(raPerp, tangent);
    float rbPerpDotT = glm::dot(rbPerp, tangent);

    float denom = boxInvMass1 + boxInverseMass2 +
      (raPerpDotT * raPerpDotT) * boxInvInertia1 +
      (rbPerpDotT * rbPerpDotT) * boxInverseInertia2;

    float contactVelocityMagnitude = glm::dot(relativeVelocity, tangent);

    float jt = -contactVelocityMagnitude;
    jt /= denom;

    jt /= static_cast<float>(m.nContacts);

    glm::vec3 frictionImpulse = jt * tangent;

    if (std::abs(jt) > jList[i] * sf) {
      frictionImpulse = -jList[i] * tangent * df;
    }

    frictionImpulseList[i] = frictionImpulse;
  }

  for (int i = 0; i < m.nContacts; i++) {
    if (ignoreContact[i]) continue;
    glm::vec3 frictionImpulse = frictionImpulseList[i];

    boxLinearVelocity1 += -frictionImpulse * boxInvMass1;
    boxAngularVelocity1 += -Transformations::cross(glm::vec2(raList[i].x, raList[i].y), glm::vec2(frictionImpulse.x, frictionImpulse.y)) * boxInvInertia1;
    boxLinearVelocity2 += frictionImpulse * boxInverseMass2;
    boxAngularVelocity2 += Transformations::cross(glm::vec2(rbList[i].x, rbList[i].y), glm::vec2(frictionImpulse.x, frictionImpulse.y)) * boxInverseInertia2;
  }

}

float PhysicsEngine::calculateMomentOfInertia(const glm::vec3 &min, const glm::vec3 &max, float mass) {
  float width = max.x - min.x;
  float height = max.y - min.y;

  float momentOfInertia = (mass / 12.0f) * (width * width + height * height);

  return momentOfInertia;
}


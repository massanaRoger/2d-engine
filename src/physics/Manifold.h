#ifndef MANIFOLD_H
#define MANIFOLD_H
#include "glm/vec3.hpp"
#include <vector>

struct Manifold {
    glm::vec3 normal;
    float penetration;
    glm::vec3 contactPoint1;
    glm::vec3 contactPoint2;
    int nContacts;

    void ApplyPositionalCorrection(glm::vec3& positionA, glm::vec3& positionB, float invMassA, float invMassB) const;
    bool CirclevsCircle(const glm::vec3 &centerA, float radiusA, const glm::vec3 &centerB, float radiusB);
    bool BoxvsBox(const std::vector<glm::vec3> &boxVerticesA, const glm::vec3 &boxCenterA, const std::vector<glm::vec3> &boxVerticesB, const glm::vec3 &boxCenterB);
    bool CirclevsBox(const glm::vec3 &circleCenter, float circleRadius, const std::vector<glm::vec3> &boxVertices, const glm::vec3 &boxCenter);
};

#endif

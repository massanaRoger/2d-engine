#include "Manifold.h"
#include "../utils.h"
#include <limits>
#define GLM_ENABLE_EXPERIMENTAL

#include "contacts.h"
#include "glm/gtx/norm.hpp"

void Manifold::ApplyPositionalCorrection(glm::vec3& positionA, glm::vec3& positionB, float invMassA, float invMassB) const {
    const float percent = 0.2f;
    const float slop = 0.01f;
    float totalInvMass = invMassA + invMassB;
    glm::vec3 correction = std::max(penetration - slop, 0.0f) / totalInvMass * percent * normal;
    positionA -= correction * invMassA;
    positionB += correction * invMassB;
}

bool Manifold::CirclevsCircle(const glm::vec3 &centerA, float radiusA, const glm::vec3 &centerB, float radiusB) {
    glm::vec3 ab = centerB - centerA;
    float r = radiusA + radiusB;

    r *= r;
    if (glm::length2(ab) > r) return false;

    float d = glm::length(ab);
    penetration = r - d;
    normal = ab / d;
    if (glm::dot(ab, normal) < 0.0f) {
        normal = -normal;
    }

    contactPoint1 = contactPointCircleCircle(centerA, radiusA, centerB);
    nContacts = 1;
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
        axis = glm::normalize(axis);

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
    axis = glm::normalize(axis);

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

bool Manifold::BoxvsBox(const std::vector<glm::vec3> &boxVerticesA, const glm::vec3 &boxCenterA, const std::vector<glm::vec3> &boxVerticesB, const glm::vec3 &boxCenterB) {
    normal = glm::vec3(1.0f);
    penetration = std::numeric_limits<float>::max();

    for (int i = 0; i < boxVerticesA.size(); i++) {
        glm::vec3 va = boxVerticesA[i];
        glm::vec3 vb = boxVerticesA[(i + 1) % boxVerticesA.size()];

        glm::vec3 edge = vb - va;
        glm::vec3 axis(-edge.y, edge.x, 0.0f);
        axis = glm::normalize(axis);

        float minA, maxA;
        float minB, maxB;
        projectPolygon(boxVerticesA, axis, minA, maxA);
        projectPolygon(boxVerticesB, axis, minB, maxB);

        if (minA >= maxB || minB >= maxA) {
            return false;
        }

        float axisDepth = std::min(maxB - minA, maxA - minB);
        if (axisDepth < penetration) {
            penetration = axisDepth;
            normal = axis;
        }
    }
    for (int i = 0; i < boxVerticesB.size(); i++) {
        glm::vec3 va = boxVerticesB[i];
        glm::vec3 vb = boxVerticesB[(i + 1) % boxVerticesB.size()];

        glm::vec3 edge = vb - va;
        glm::vec3 axis(-edge.y, edge.x, 0.0f);
        axis = glm::normalize(axis);

        float minA, maxA;
        float minB, maxB;
        projectPolygon(boxVerticesA, axis, minA, maxA);
        projectPolygon(boxVerticesB, axis, minB, maxB);

        if (minA >= maxB || minB >= maxA) {
            return false;
        }

        float axisDepth = std::min(maxB - minA, maxA - minB);
        if (axisDepth < penetration) {
            penetration = axisDepth;
            normal = axis;
        }
    }

    penetration /= glm::length(normal);
    normal = glm::normalize(normal);

    glm::vec3 direction = boxCenterB - boxCenterA;

    if (glm::dot(direction, normal) < 0.0f) {
        normal = -normal;
    }

    ContactPoints contactPoints = contactPointsBoxBox(boxVerticesA, boxVerticesB);

    contactPoint1 = contactPoints.contact1;
    contactPoint2 = contactPoints.contact2;
    nContacts = contactPoints.nContacts;

    return true;
}



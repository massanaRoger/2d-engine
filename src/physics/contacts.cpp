#include <cmath>

#include "glm/vec3.hpp"
#include "glm/detail/func_geometric.inl"
#include "glm/gtx/norm.inl"
#include "contacts.h"


// Value to account for floating point innacuracies
static constexpr float inaccuracyCheck = 0.0005f;

ContactInfo pointSegmentDistance(const glm::vec3 &p, const glm::vec3 &a, const glm::vec3 &b) {
    glm::vec3 ab = b - a;
    glm::vec3 ap = p - a;

    float proj = glm::dot(ap, ab);
    float abLenSq = glm::length2(ab);
    float d = proj / abLenSq;

    if (d <= 0.0f) {
        return ContactInfo {
            a,
            glm::distance2(p, a)
        };
    }
    if (d >= 1.0f) {
        return ContactInfo {
            b,
            glm::distance2(p, b)
        };
    }
    return ContactInfo {
        a + ab * d,
        glm::distance2(p, a + ab * d)
    };
}

glm::vec3 contactPointCirclevsBox(const glm::vec3 &circleCenter, const std::vector<glm::vec3> &boxVertices) {
    float minDistSq = std::numeric_limits<float>::max();
    glm::vec3 contactPoint{};

    for (int i = 0; i < boxVertices.size(); i++) {
        glm::vec3 va = boxVertices[i];
        glm::vec3 vb = boxVertices[(i + 1) % boxVertices.size()];

        ContactInfo contactInfo = pointSegmentDistance(circleCenter, va, vb);

        if (contactInfo.distanceSquared < minDistSq) {
            minDistSq = contactInfo.distanceSquared;
            contactPoint = contactInfo.contact;
        }
    }
    return contactPoint;
}

ContactPoints contactPointsBoxBox(const std::vector<glm::vec3> &verticesA, const std::vector<glm::vec3> &verticesB) {
    ContactPoints result {};
    float minDistSq = std::numeric_limits<float>::max();

    for (int i = 0; i < verticesA.size(); i++) {
        glm::vec3 p = verticesA[i];

        for (int j = 0; j < verticesB.size(); j++) {
            glm::vec3 va = verticesB[j];
            glm::vec3 vb = verticesB[(j + 1) % verticesB.size()];

            ContactInfo contactInfo = pointSegmentDistance(p, va, vb);
            if (std::abs(glm::length(contactInfo.distanceSquared - minDistSq)) < inaccuracyCheck) {
                if (std::abs(glm::length(contactInfo.contact - result.contact1)) > inaccuracyCheck) {
                    result.contact2 = contactInfo.contact;
                    result.nContacts = 2;
                }
            } else if (contactInfo.distanceSquared < minDistSq) {
                minDistSq = contactInfo.distanceSquared;
                result.nContacts = 1;
                result.contact1 = contactInfo.contact;
            }
        }
    }

    for (int i = 0; i < verticesB.size(); i++) {
        glm::vec3 p = verticesB[i];

        for (int j = 0; j < verticesA.size(); j++) {
            glm::vec3 va = verticesA[j];
            glm::vec3 vb = verticesA[(j + 1) % verticesA.size()];

            ContactInfo contactInfo = pointSegmentDistance(p, va, vb);
            if (std::abs(glm::length(contactInfo.distanceSquared - minDistSq)) < inaccuracyCheck) {
                if (std::abs(glm::length(contactInfo.contact - result.contact1)) > inaccuracyCheck) {
                    result.contact2 = contactInfo.contact;
                    result.nContacts = 2;
                }
            } else if (contactInfo.distanceSquared < minDistSq) {
                minDistSq = contactInfo.distanceSquared;
                result.nContacts = 1;
                result.contact1 = contactInfo.contact;
            }
        }
    }

    return result;
}

glm::vec3 contactPointCircleCircle(const glm::vec3 &centerA, float radiusA, const glm::vec3 &centerB) {
    glm::vec3 contactPoint{};
    glm::vec3 ab = centerB - centerA;
    glm::vec3 dir = glm::normalize(ab);
    contactPoint = centerA + dir * radiusA;
    return contactPoint;
}

int findClosestPointOnPolygon(const glm::vec3 &circleCenter, const std::vector<glm::vec3> &vertices) {
    int result = -1;
    float minDistance = std::numeric_limits<float>::max();

    for (int i = 0; i < vertices.size(); i++) {
        glm::vec3 v = vertices[i];
        float distance = glm::distance(v, circleCenter);

        if (distance < minDistance) {
            minDistance = distance;
            result = i;
        }
    }

    return result;
}
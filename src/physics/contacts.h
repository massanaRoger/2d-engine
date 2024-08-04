#pragma once
#include <vector>

#include "glm/vec3.hpp"

struct ContactInfo {
    glm::vec3 contact;
    float distanceSquared;
};

ContactInfo pointSegmentDistance(const glm::vec3 &p, const glm::vec3 &a, const glm::vec3 &b);
glm::vec3 contactPointCirclevsBox(const glm::vec3 &circleCenter, const std::vector<glm::vec3> &boxVertices);
int findClosestPointOnPolygon(const glm::vec3 &circleCenter, const std::vector<glm::vec3> &vertices);
glm::vec3 contactPointCircleCircle(const glm::vec3 &centerA, float radiusA, const glm::vec3 &centerB);
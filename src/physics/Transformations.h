#pragma once

#include <vector>
#include "glm/glm.hpp"

namespace Transformations {
    void updateMatrix(glm::mat4 &transformationMatrix, const glm::vec3 &centerOfMass, float rotation);
    glm::mat4 createProjectionMatrix(int width, int height);
    std::vector<glm::vec3> getWorldVertices(const std::vector<glm::vec3> &localVertices, const glm::mat4 &transformationMatrix);
    float cross(const glm::vec2& a, const glm::vec2& b);
    float calculateCircleInertia(float mass, float radius);
    float calculateBoxInertia(float mass, float width, float height);

}

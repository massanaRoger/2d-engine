#include "Transformations.h"

#include <iostream>

#include "glm/ext/matrix_transform.hpp"

void Transformations::updateMatrix(glm::mat4 &transformationMatrix, const glm::vec3 &centerOfMass, float rotation) {
    transformationMatrix = glm::mat4(1.0f);
    transformationMatrix = glm::translate(transformationMatrix, centerOfMass);
    transformationMatrix = glm::rotate(transformationMatrix, rotation, glm::vec3(0.0, 0.0, 1.0));
}

std::vector<glm::vec3> Transformations::getWorldVertices(const std::vector<glm::vec3> &localVertices, const glm::mat4 &transformationMatrix) {
    std::vector<glm::vec3> worldVertices;
    worldVertices.reserve(localVertices.size());
    for (const auto &vertex : localVertices) {
        glm::vec4 transformedVertex = transformationMatrix * glm::vec4(vertex, 1.0f);
        worldVertices.emplace_back(transformedVertex);
    }
    return worldVertices;
}

float Transformations::cross(const glm::vec2& a, const glm::vec2& b) {
    return a.x * b.y - a.y * b.x;
}

float Transformations::calculateCircleInertia(float mass, float radius) {
    std::cout << "Radius: " << radius << std::endl;
    return 0.5f * mass * radius * radius;
}

float Transformations::calculateBoxInertia(float mass, float width, float height) {
    std::cout << "Width: " << width << std::endl;
    std::cout << "Height: " << height << std::endl;

    return (1.0f / 12.0f) * mass * (width * width + height * height);
}

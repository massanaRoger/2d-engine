
#include <glad/glad.h>
#include <glm/glm.hpp>

#include "Polygon.h"
#include "glm/ext/matrix_transform.hpp"
#include "shader/Shader.h"

void Polygon::draw(Shader &shader) {
    auto transform = glm::mat4(1.0f);

    transform = glm::translate(transform, transVertices);
    transform = glm::rotate(transform, glm::radians(angle), glm::vec3(0.0, 0.0, 1.0));

    transformMatrix = transform;

    GLint transformLoc = glGetUniformLocation(shader.programID, "transform");
    glUniformMatrix4fv(transformLoc, 1, GL_FALSE, &transform[0][0]);

    shader.setInt("u_objType", 2);
}

void Polygon::update(float deltaTime) {
    transVertices = transVertices + velocity * deltaTime;
    velocity = velocity * std::pow(damping, deltaTime) + acceleration * deltaTime;
}

Polygon::Polygon(std::initializer_list<glm::vec3> il, const glm::vec3 &velocity, const glm::vec3 &acceleration, float damping) : vertices(il), velocity(velocity), acceleration(acceleration), damping(damping), transformMatrix(glm::mat4(1.0f)) {}

Polygon::Polygon(std::vector<glm::vec3>&& vertices, const glm::vec3 &velocity, const glm::vec3 &acceleration, float damping) : vertices(vertices), velocity(velocity), acceleration(acceleration), damping(damping), transformMatrix(glm::mat4(1.0f)) {}

[[nodiscard]] ObjectType Polygon::getType() const {
    return ObjectType::Polygon;
}

[[nodiscard]] std::vector<glm::vec3> Polygon::transformedVertices() const {
    std::vector<glm::vec3> transformedVertices;
    transformedVertices.reserve(vertices.size());
    for (auto &vert : vertices) {
        glm::vec4 transformedVert = transformMatrix * glm::vec4(vert, 1.0f);
        transformedVertices.emplace_back(transformedVert);
    }
    return transformedVertices;
};

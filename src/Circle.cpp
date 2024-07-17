#include "Circle.h"

#include <glad/glad.h>

#include <cmath>

#include "glm/ext/vector_float3.hpp"
#include "glm/glm.hpp"
#include "glm/ext/matrix_transform.hpp"

Circle::Circle(std::size_t n, float radius, const glm::vec3 &position, const glm::vec3 &velocity, const glm::vec3 &acceleration, float mass, float damping): radius(radius), position(position), velocity(velocity), acceleration(acceleration), mass(mass), damping(damping) {
    inverseMass = 1 / mass;
}

void Circle::draw(Shader& shader) {
    auto transform = glm::mat4(1.0f);
    transform = glm::translate(transform, position);

    GLint transformLoc = glGetUniformLocation(shader.programID, "transform");
    glUniformMatrix4fv(transformLoc, 1, GL_FALSE, &transform[0][0]);

    shader.setVec2("u_center", position.x, position.y);
    shader.setFloat("u_radius", radius);
    shader.setInt("u_objType", 1);
}

void Circle::update(float deltaTime) {
    position = position + velocity * deltaTime;
    velocity = velocity * std::pow(damping, deltaTime) + acceleration * deltaTime;
}

[[nodiscard]] ObjectType Circle::getType() const {
    return ObjectType::Circle;
}


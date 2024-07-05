#include "AABB.h"
#include "glm/fwd.hpp"
#include <glad/glad.h>
#include "glm/glm.hpp"

AABB::AABB(glm::vec2 &min, glm::vec2 &max): m_min(min), m_max(max) {}
AABB::AABB(float minX, float minY, float maxX, float maxY): m_min(minX, minY), m_max(maxX, maxY) {}

void AABB::update(float deltaTime) {}
void AABB::draw(Shader& shader) {
    auto transform = glm::mat4(1.0f);

    GLint transformLoc = glGetUniformLocation(shader.programID, "transform");
    glUniformMatrix4fv(transformLoc, 1, GL_FALSE, &transform[0][0]);

    shader.setVec2("u_min", m_min.x, m_min.y);
    shader.setVec2("u_max", m_max.x, m_max.y);
    shader.setInt("u_objType", 0);
}

[[nodiscard]] ObjectType AABB::getType() const {
    return ObjectType::AABB;
}

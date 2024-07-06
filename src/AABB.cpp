#include "AABB.h"
#include "glm/fwd.hpp"
#include <glad/glad.h>
#include "glm/glm.hpp"

AABB::AABB(glm::vec2 &min, glm::vec2 &max): min(min), max(max) {}
AABB::AABB(float minX, float minY, float maxX, float maxY): min(minX, minY), max(maxX, maxY) {}

void AABB::update(float deltaTime) {}
void AABB::draw(Shader& shader) {
    auto transform = glm::mat4(1.0f);

    GLint transformLoc = glGetUniformLocation(shader.programID, "transform");
    glUniformMatrix4fv(transformLoc, 1, GL_FALSE, &transform[0][0]);

    shader.setVec2("u_min", min.x, min.y);
    shader.setVec2("u_max", max.x, max.y);
    shader.setInt("u_objType", 0);
}

[[nodiscard]] ObjectType AABB::getType() const {
    return ObjectType::AABB;
}

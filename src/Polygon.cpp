
#include <glad/glad.h>
#include <glm/glm.hpp>

#include "Polygon.h"
#include "glm/ext/matrix_transform.hpp"
#include "shader/Shader.h"

void Polygon::draw(Shader &shader) {
    auto transform = glm::mat4(1.0f);

    GLint transformLoc = glGetUniformLocation(shader.programID, "transform");
    glUniformMatrix4fv(transformLoc, 1, GL_FALSE, &transform[0][0]);

    shader.setInt("u_objType", 2);
}

void Polygon::update(float deltaTime) {

}

Polygon::Polygon(std::initializer_list<glm::vec3> il) : vertices(il) {}

Polygon::Polygon(std::vector<glm::vec3>&& vertices) : vertices(vertices) {}

[[nodiscard]] ObjectType Polygon::getType() const {
    return ObjectType::Polygon;
}

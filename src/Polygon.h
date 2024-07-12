#ifndef BOX2D_H
#define BOX2D_H

#include "Object.h"
#include <glm/glm.hpp>
#include <initializer_list>
#include <vector>

class Polygon : public Object {
public:
    float angle = 0.0f;
    glm::vec3 transVertices = glm::vec3(0.0f, 0.0f, 0.0f);
    glm::vec3 velocity;
    glm::vec3 acceleration;
    float damping;
    glm::mat4 transformMatrix = glm::mat4(1.0f);
    std::vector<glm::vec3> vertices;

    Polygon(std::initializer_list<glm::vec3> il, const glm::vec3 &velocity, const glm::vec3 &acceleration, float damping);
    Polygon(std::vector<glm::vec3>&& vertices, const glm::vec3 &velocity, const glm::vec3 &acceleration, float damping);
    [[nodiscard]] ObjectType getType() const override;
    void draw(Shader& shader) override;
    void update(float deltaTime) override;
    [[nodiscard]] std::vector<glm::vec3> transformedVertices() const;
};

#endif

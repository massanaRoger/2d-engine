#ifndef BOX2D_H
#define BOX2D_H

#include "Object.h"
#include "glm/ext/vector_float3.hpp"
#include <initializer_list>
#include <vector>
class Polygon : public Object {
public:
    std::vector<glm::vec3> vertices;
    float angle = 0.0f;
    glm::vec3 transVertices = glm::vec3(0.0f, 0.0f, 0.0f);
    glm::vec3 velocity;
    glm::vec3 acceleration;
    float damping;

    Polygon(std::initializer_list<glm::vec3> il, const glm::vec3 &velocity, const glm::vec3 &acceleration, float damping);
    Polygon(std::vector<glm::vec3>&& vertices, const glm::vec3 &velocity, const glm::vec3 &acceleration, float damping);
    [[nodiscard]] ObjectType getType() const override;
    void draw(Shader& shader) override;
    void update(float deltaTime) override;
};

#endif

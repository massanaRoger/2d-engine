#ifndef CIRCLE_H
#define CIRCLE_H

#include <vector>

#include "glm/ext/vector_float3.hpp"
#include "glm/vec3.hpp"
#include "Object.h"

class Circle: public Object {
public:
    glm::vec3 position;
    glm::vec3 velocity;
    glm::vec3 acceleration;
    float radius;
    float mass;
    float inverseMass;
    float damping;

    explicit Circle(std::size_t n, float radius, const glm::vec3 &position, const glm::vec3 &velocity, const glm::vec3 &acceleration, float mass, float damping);

    void draw(Shader& shader) override;
    void update(float deltaTime) override;
    [[nodiscard]] ObjectType getType() const override;

};
#endif

#ifndef CIRCLE_H
#define CIRCLE_H

#include <vector>

#include "glm/ext/vector_float3.hpp"
#include "glm/vec3.hpp"

class Circle {
public:
    glm::vec3 position;
    glm::vec3 velocity;
    float radius;
    float mass;
    float inverseMass;

    explicit Circle(std::size_t n, float radius, const glm::vec3 &position, const glm::vec3 &velocity, float mass);

    static bool checkCollision(const Circle &circle1, const Circle &circle2);
    static void resolveCollision(Circle &circle1, Circle &circle2);

};
#endif

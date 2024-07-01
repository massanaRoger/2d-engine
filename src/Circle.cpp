#include "Circle.h"
#include "glm/ext/vector_float3.hpp"
#include "glm/geometric.hpp"

Circle::Circle(std::size_t n, float radius, const glm::vec3 &position, const glm::vec3 &velocity, float mass) : radius(radius), position(position), velocity(velocity), mass(mass) {
    inverseMass = 1 / mass;
}

bool Circle::checkCollision(const Circle &circle1, const Circle &circle2) {
    glm::vec3 distance = circle1.position - circle2.position; 
    float distanceSquared = distance.x * distance.x + distance.y * distance.y;
    float radiusSum = circle1.radius + circle2.radius;
    return distanceSquared < (radiusSum * radiusSum);
}

void Circle::resolveCollision(Circle &circle1, Circle &circle2) {
    glm::vec3 collisionNormal = circle2.position - circle1.position;
    glm::vec3 relativeVelocity = circle2.velocity - circle1.velocity;

    float velocityAlongNormal = glm::dot(relativeVelocity, collisionNormal);
    if (velocityAlongNormal > 0) {
        return;
    }

    // Calculate restitution
    float e = 1.0f;
    float j = -(1 + e) * velocityAlongNormal;
    j /= (1 / circle1.mass + 1 / circle2.mass);

    glm::vec3 impulse = collisionNormal * j;
    circle1.velocity = circle1.velocity - impulse * circle1.inverseMass;
    circle2.velocity = circle2.velocity - impulse * circle2.inverseMass;
}

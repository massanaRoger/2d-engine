//
// Created by Roger2 on 27/06/2024.
//

#include "PhysicsEngine.h"

#include "glm/detail/func_geometric.inl"

void PhysicsEngine::update(std::vector<Object*>* objects, float deltaTime) {
    for (auto& obj : *objects) {
        obj->update(deltaTime);
    }

    for (size_t i = 0; i < objects->size(); ++i) {
        for (size_t j = i + 1; j < objects->size(); ++j) {
            auto obj1 = (*objects)[i];
            auto obj2 = (*objects)[j];
            if (obj1->getType() == ObjectType::Circle && obj2->getType() == ObjectType::Circle) {
                auto* circle1 = static_cast<Circle*>(obj1);
                auto* circle2 = static_cast<Circle*>(obj2);

                if (checkCollisionCircleCircle(*circle1, *circle2)) {
                    resolveCollisionCircleCircle(*circle1, *circle2);
                }
            }
        }
    }
}

bool PhysicsEngine::checkCollisionCircleCircle(const Circle &circle1, const Circle &circle2) {
    glm::vec3 distance = circle1.position - circle2.position;
    float distanceSquared = distance.x * distance.x + distance.y * distance.y;
    float radiusSum = circle1.radius + circle2.radius;
    return distanceSquared < (radiusSum * radiusSum);
}

void PhysicsEngine::resolveCollisionCircleCircle(Circle &circle1, Circle &circle2) {
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
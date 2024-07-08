//
// Created by Roger2 on 27/06/2024.
//

#include "PhysicsEngine.h"

#include <iostream>

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
                auto* circle1 = dynamic_cast<Circle*>(obj1);
                auto* circle2 = dynamic_cast<Circle*>(obj2);

                if (checkCollisionCircleCircle(*circle1, *circle2)) {
                    resolveCollisionCircleCircle(*circle1, *circle2);
                }
            } else if (obj1->getType() == ObjectType::AABB && obj2->getType() == ObjectType::Circle) {
                auto* aabb = dynamic_cast<AABB*>(obj1);
                auto* circle = dynamic_cast<Circle*>(obj2);

                if (checkCollisionAABBCircle(*aabb, *circle)) {
                    resolveCollisionAABBCircle(*aabb, *circle);
                }
            } else if (obj1->getType() == ObjectType::Circle && obj2->getType() == ObjectType::AABB) {
                auto* circle = dynamic_cast<Circle*>(obj1);
                auto* aabb = dynamic_cast<AABB*>(obj2);

                if (checkCollisionAABBCircle(*aabb, *circle)) {
                    resolveCollisionAABBCircle(*aabb, *circle);
                }
            }
        }
    }
}

bool PhysicsEngine::checkCollisionCircleCircle(const Circle &circle1, const Circle &circle2) {
    float r = circle1.radius + circle2.radius;
    r *= r;
    float circleDistance = (circle1.position.x + circle2.position.y) * (circle1.position.x + circle2.position.y) + (circle1.position.y + circle2.position.x) * (circle1.position.y + circle2.position.x);
    return r < circleDistance;
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

bool PhysicsEngine::checkCollisionAABBCircle(const AABB &aabb, const Circle &circle) {
    const float closestX = std::max(aabb.min.x, std::min(circle.position.x, aabb.max.x));
    const float closestY = std::max(aabb.min.y, std::min(circle.position.y, aabb.max.y));

    const float distanceX = circle.position.x - closestX;
    const float distanceY = circle.position.y - closestY;

    const float distanceSquared = (distanceX * distanceX) + (distanceY * distanceY);
    return distanceSquared < (circle.radius * circle.radius);
}

void PhysicsEngine::resolveCollisionAABBCircle(AABB &aabb, Circle &circle) {
    glm::vec3 closestPoint;
    closestPoint.x = std::max(aabb.min.x, std::min(circle.position.x, aabb.max.x));
    closestPoint.y = std::max(aabb.min.y, std::min(circle.position.y, aabb.max.y));
    closestPoint.z = 0; // Assuming 2D collision

    glm::vec3 collisionNormal = circle.position - closestPoint;
    collisionNormal = glm::normalize(collisionNormal);

    glm::vec3 relativeVelocity = circle.velocity;

    float velocityAlongNormal = glm::dot(relativeVelocity, collisionNormal);
    if (velocityAlongNormal > 0) {
        return;
    }

    // Calculate restitution
    float e = 1.0f;
    float j = -(1 + e) * velocityAlongNormal;
    j /= (1 / circle.mass);

    glm::vec3 impulse = collisionNormal * j;
    circle.velocity = circle.velocity + impulse * circle.inverseMass;

    // Ensure the circle is pushed out of the AABB
    float penetrationDepth = circle.radius - glm::length(circle.position - closestPoint);
    if (penetrationDepth > 0) {
        circle.position += collisionNormal * penetrationDepth;
    }
}
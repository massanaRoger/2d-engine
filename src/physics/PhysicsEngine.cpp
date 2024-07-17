//
// Created by Roger2 on 27/06/2024.
//

#include "PhysicsEngine.h"

#include "Manifold.h"
#include "../Polygon.h"

#include "../utils.h"
#include <iostream>

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
                    Manifold manifold{};
                    manifold.AABBvsCircle(*aabb, *circle);
                    resolveCollisionAABBCircle(manifold);
                }
            } else if (obj1->getType() == ObjectType::Circle && obj2->getType() == ObjectType::AABB) {
                auto* circle = dynamic_cast<Circle*>(obj1);
                auto* aabb = dynamic_cast<AABB*>(obj2);

                if (checkCollisionAABBCircle(*aabb, *circle)) {
                    Manifold manifold{};
                    manifold.AABBvsCircle(*aabb, *circle);
                    resolveCollisionAABBCircle(manifold);
                }
            } else if (obj1->getType() == ObjectType::Polygon && obj2->getType() == ObjectType::AABB) {
                auto* polygon = dynamic_cast<Polygon*>(obj1);
                auto* aabb = dynamic_cast<AABB*>(obj2);

                if (checkCollisionPolygonAABB(*polygon, *aabb)) {
                    Manifold manifold{};
                    manifold.PolygonvsAABB(*polygon, *aabb);
                    resolveCollisionPolygonAABB(manifold);
                }
            } else if (obj1->getType() == ObjectType::AABB && obj2->getType() == ObjectType::Polygon) {
                auto* aabb = dynamic_cast<AABB*>(obj1);
                auto* polygon = dynamic_cast<Polygon*>(obj2);

                if (checkCollisionPolygonAABB(*polygon, *aabb)) {
                    Manifold manifold{};
                    manifold.PolygonvsAABB(*polygon, *aabb);
                    resolveCollisionPolygonAABB(manifold);
                }
            }


        }
    }
}

void PhysicsEngine::resolveCollisionPolygonAABB(Manifold &m) {
    auto polygon = dynamic_cast<Polygon*>(m.A);
    auto aabb = dynamic_cast<AABB*>(m.B);

    glm::vec3 collisionNormal = m.normal;
    float penetration = m.penetration;

    // Calculate relative velocity
    glm::vec3 relativeVelocity = polygon->velocity;

    float velocityAlongNormal = glm::dot(relativeVelocity, collisionNormal);
    if (velocityAlongNormal > 0) {
        return;
    }

    // Calculate restitution
    float e = 1.0f;  // Assume elasticity (restitution)
    float j = -(1 + e) * velocityAlongNormal;
    j /= polygon->inverseMass;

    glm::vec3 impulse = collisionNormal * j;
    polygon->velocity += impulse * polygon->inverseMass;

    // Calculate the point of collision (assuming center of mass for simplicity)
    glm::vec3 r = collisionNormal * penetration;
    glm::vec3 angularImpulse = glm::cross(r, impulse);

    // Apply angular impulse
    polygon->angularVelocity += angularImpulse.z * polygon->inverseInertia;

    // Positional correction to avoid sinking
    const float percent = 0.8f; // usually 20% to 80%
    const float slop = 0.01f; // usually 0.01 to 0.1
    glm::vec3 correction = std::max(penetration - slop, 0.0f) / polygon->inverseMass * percent * collisionNormal;
    polygon->transVertices += correction;
}

bool PhysicsEngine::checkCollisionCircleCircle(const Circle &circle1, const Circle &circle2) {
    float r = circle1.radius + circle2.radius;
    r *= r;
    float dx = circle1.position.x - circle2.position.x;
    float dy = circle1.position.y - circle2.position.y;
    float circleDistance = dx * dx + dy * dy;
    return circleDistance < r;
}


void PhysicsEngine::resolveCollisionCircleCircle(Circle &circle1, Circle &circle2) {
    glm::vec3 collisionNormal = circle2.position - circle1.position;
    collisionNormal = glm::normalize(collisionNormal);
    glm::vec3 relativeVelocity = circle2.velocity - circle1.velocity;

    float velocityAlongNormal = glm::dot(relativeVelocity, collisionNormal);
    if (velocityAlongNormal > 0) {
        return;
    }

    // Calculate restitution
    float e = 1.0f;  // You might want to make this a property of the circles if different circles have different restitution values
    float j = -(1 + e) * velocityAlongNormal;
    j /= (1 / circle1.mass + 1 / circle2.mass);

    glm::vec3 impulse = collisionNormal * j;
    circle1.velocity = circle1.velocity - impulse * circle1.inverseMass;
    circle2.velocity = circle2.velocity + impulse * circle2.inverseMass;
}


bool PhysicsEngine::checkCollisionAABBCircle(const AABB &aabb, const Circle &circle) {
    const float closestX = std::max(aabb.min.x, std::min(circle.position.x, aabb.max.x));
    const float closestY = std::max(aabb.min.y, std::min(circle.position.y, aabb.max.y));

    const float distanceX = circle.position.x - closestX;
    const float distanceY = circle.position.y - closestY;

    const float distanceSquared = (distanceX * distanceX) + (distanceY * distanceY);
    return distanceSquared < (circle.radius * circle.radius);
}

void PhysicsEngine::resolveCollisionAABBCircle(Manifold &m) {
    glm::vec3 closestPoint;
    auto aabb = dynamic_cast<AABB*>(m.A);
    auto circle = dynamic_cast<Circle*>(m.B);
    closestPoint.x = std::max(aabb->min.x, std::min(circle->position.x, aabb->max.x));
    closestPoint.y = std::max(aabb->min.y, std::min(circle->position.y, aabb->max.y));
    closestPoint.z = 0; // Assuming 2D collision

    glm::vec3 collisionNormal = circle->position - closestPoint;
    collisionNormal = glm::normalize(collisionNormal);

    glm::vec3 relativeVelocity = circle->velocity;

    float velocityAlongNormal = glm::dot(relativeVelocity, collisionNormal);
    if (velocityAlongNormal > 0) {
        return;
    }

    // Calculate restitution
    float e = 1.0f;
    float j = -(1 + e) * velocityAlongNormal;
    j /= (1 / circle->mass);

    glm::vec3 impulse = collisionNormal * j;
    circle->velocity = circle->velocity + impulse * circle->inverseMass;

    // Ensure the circle is pushed out of the AABB
    float penetrationDepth = circle->radius - glm::length(circle->position - closestPoint);
    if (penetrationDepth > 0) {
        circle->position += collisionNormal * penetrationDepth;
    }
}

bool PhysicsEngine::checkCollisionPolygonPolygon(const Polygon &p1, const Polygon &p2) {
    std::vector<glm::vec3> p1Vertices = p1.transformedVertices();
    std::vector<glm::vec3> p2Vertices = p2.transformedVertices();
    std::vector<glm::vec3> normals1 = calculateNormals(p1Vertices);
    std::vector<glm::vec3> normals2 = calculateNormals(p2Vertices);

    for (auto &normal : normals1) {
        if (!overlapOnAxis(p1Vertices, p2Vertices, normal)) {
            return false;
        }
    }

    for (auto &normal : normals2) {
        if (!overlapOnAxis(p1Vertices, p2Vertices, normal)) {
            return false;
        }
    }
    return true;
}

bool PhysicsEngine::checkCollisionPolygonAABB(const Polygon &p, const AABB &aabb) {
    std::vector<glm::vec3> pVertices = p.transformedVertices();
    std::vector<glm::vec3> normals1 = calculateNormals(pVertices);

    std::vector<glm::vec3> aabbVertices = calculateAABBvertices(aabb.min, aabb.max);
    std::vector<glm::vec3> normals2 = calculateNormals(aabbVertices);

    for (auto &normal : normals1) {
        if (!overlapOnAxis(pVertices, aabbVertices, normal)) {
            return false;
        }
    }

    for (auto &normal : normals2) {
        if (!overlapOnAxis(pVertices, aabbVertices, normal)) {
            return false;
        }
    }
    return true;
}

static void positionalCorrection(Circle &circle1, Circle &circle2) {

}

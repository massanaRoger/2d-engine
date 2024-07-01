//
// Created by Roger2 on 27/06/2024.
//

#include "PhysicsEngine.h"

void PhysicsEngine::update(std::vector<Circle>* objects, float deltaTime) {
    for (auto& obj : *objects) {
        obj.position = obj.position + obj.velocity * deltaTime;
    }

    for (size_t i = 0; i < objects->size(); ++i) {
        for (size_t j = i + 1; j < objects->size(); ++j) {
            if (Circle::checkCollision((*objects)[i], (*objects)[j])) {
                Circle::resolveCollision((*objects)[i], (*objects)[j]);
            }
        }
    }
}

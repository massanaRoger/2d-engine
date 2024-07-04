#include "AABB.h"

void AABB::update(float deltaTime) {}
void AABB::draw(Shader& shader) {}

[[nodiscard]] ObjectType AABB::getType() const {
    return ObjectType::AABB;
}
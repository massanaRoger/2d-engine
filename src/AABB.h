#ifndef AABB_H
#define AABB_H
#include "glm/vec2.hpp"
#include "Object.h"

class AABB: public Object {
public:
    glm::vec2 min;
    glm::vec2 max;

    AABB(glm::vec2 &min, glm::vec2 &max);
    AABB(float minX, float minY, float maxX, float maxY);

    void draw(Shader& shader) override;
    void update(float deltaTime) override;
    [[nodiscard]] ObjectType getType() const override;
};



#endif

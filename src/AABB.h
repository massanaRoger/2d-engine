#ifndef AABB_H
#define AABB_H
#include "glm/vec2.hpp"
#include "Object.h"

class AABB: public Object {
public:
    AABB(glm::vec2 &min, glm::vec2 &max);
    AABB(float minX, float minY, float maxX, float maxY);

    void draw(Shader& shader) override;
    void update(float deltaTime) override;
    [[nodiscard]] ObjectType getType() const override;
private:
    glm::vec2 m_min;
    glm::vec2 m_max;
};



#endif

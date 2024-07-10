#ifndef OBJECT_H
#define OBJECT_H

#include "shader/Shader.h"

enum class ObjectType {
    Circle,
    AABB,
    Polygon,
};

class Object {
public:
    virtual ~Object() = default;
    [[nodiscard]] virtual ObjectType getType() const = 0;
    virtual void draw(Shader& shader) = 0;
    virtual void update(float deltaTime) = 0;
};

#endif

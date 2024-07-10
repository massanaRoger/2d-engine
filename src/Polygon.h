#ifndef BOX2D_H
#define BOX2D_H

#include "Object.h"
#include "glm/ext/vector_float3.hpp"
#include <initializer_list>
#include <vector>
class Polygon : public Object {
public:
    std::vector<glm::vec3> vertices;

    Polygon(std::initializer_list<glm::vec3> il);
    [[nodiscard]] ObjectType getType() const override;
    void draw(Shader& shader) override;
    void update(float deltaTime) override;
};

#endif

#pragma once

#include "glm/glm.hpp"

extern int s_componentCounter;

template <class T>
int GetId() {
    static int s_componentId = s_componentCounter++;
    return s_componentId;
}

struct TransformComponent {
    glm::vec2 position;
};

struct Shape {
    glm::vec3 color;
};


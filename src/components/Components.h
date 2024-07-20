#pragma once

#include <vector>

#include "glm/glm.hpp"

struct PositionComponent {
    glm::vec3 position{0.0f, 0.0f, 0.0f};
};

struct VelocityComponent {
    glm::vec3 velocity{0.0f, 0.0f, 0.0f};
};

struct AccelerationComponent {
    glm::vec3 acceleration{0.0f, 0.0f, 0.0f};
};

struct MassComponent {
    float inverseMass{0.0f};
};

struct CircleComponent {
    float radius{0.0f};
};

struct AABBComponent {
    glm::vec3 min{0.0f};
    glm::vec3 max{0.0f};
};

struct PolygonComponent {
    std::vector<glm::vec3> vertices{};
    float rotation;
};

struct AngularVelocityComponent {
    float angularVelocity;
};

struct AngularAccelerationComponent {
    float angularAcceleration;
};

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

struct BoxComponent {
    std::vector<glm::vec3> vertices;
};

struct CenterOfMassComponent {
    glm::vec3 centerOfMass{0.0f};
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

struct InertiaComponent {
    float invInertia;
};

struct OrientationComponent {
    float orientation;
};

struct TransformComponent {
    glm::mat4 transformMatrix;
};

struct FrictionComponent {
    float staticFriction;
    float dynamicFriction;
};

struct MovingComponent {};

struct ColorComponent {
    glm::vec4 color;
};
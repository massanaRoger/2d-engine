#pragma once
#include "glm/vec3.hpp"

class Box {
public:
    glm::vec3 *min;
    glm::vec3 *max;
    float *inverseMass;
    glm::vec3 *centerOfMass;
    glm::vec3 *velocity;
    glm::vec3 *acceleration;
    float *angularVelocity;
    float *angularAcceleration;
    float *inertia;
    float *orientation;

    Box(glm::vec3 *min, glm::vec3 *max, float *inverse_mass, glm::vec3 *center_of_mass, glm::vec3 *velocity,
        glm::vec3 *acceleration, float *angular_velocity, float *angular_acceleration, float *inertia,
        float *orientation);

};


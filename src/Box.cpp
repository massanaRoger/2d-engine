//
// Created by Roger2 on 02/08/2024.
//

#include "Box.h"

Box::Box(glm::vec3 *min, glm::vec3 *max, float *inverse_mass, glm::vec3 *center_of_mass, glm::vec3 *velocity,
        glm::vec3 *acceleration, float *angular_velocity, float *angular_acceleration, float *inertia,
        float *orientation)
        : min(min),
          max(max),
          inverseMass(inverse_mass),
          centerOfMass(center_of_mass),
          velocity(velocity),
          acceleration(acceleration),
          angularVelocity(angular_velocity),
          angularAcceleration(angular_acceleration),
          inertia(inertia),
          orientation(orientation) {
}
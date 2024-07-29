#ifndef CIRCLE_H
#define CIRCLE_H

#include <vector>

#include "glm/ext/vector_float3.hpp"
#include "glm/vec3.hpp"
#include "Object.h"
#include "components/Components.h"

struct Circle {
    PositionComponent *pc;
    VelocityComponent *vc;
    AccelerationComponent *ac;
    CircleComponent *cc;
    MassComponent *mc;

    Circle(PositionComponent *pos, VelocityComponent *vel, AccelerationComponent *acc, MassComponent *mass, CircleComponent *circle);
};
#endif

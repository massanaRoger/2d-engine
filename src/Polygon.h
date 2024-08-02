#ifndef BOX2D_H
#define BOX2D_H

#include "components/Components.h"

struct Polygon {
    VelocityComponent *vc;
    AccelerationComponent *ac;
    MassComponent *mc;
    PolygonComponent *pc;
    AngularVelocityComponent *avc;
    AngularAccelerationComponent *aac;
    InertiaComponent *ic;

    Polygon(VelocityComponent *vc, AccelerationComponent *ac, MassComponent *mc, PolygonComponent *pc, AngularVelocityComponent *avc, AngularAccelerationComponent *aac, InertiaComponent *ic);

    static float calculateRotationalInertia(const std::vector<glm::vec3> &vertices, float inverseMass);

};

#endif

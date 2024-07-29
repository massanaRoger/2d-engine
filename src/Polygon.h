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

    Polygon(VelocityComponent *vc, AccelerationComponent *ac, MassComponent *mc, PolygonComponent *pc, AngularVelocityComponent *avc, AngularAccelerationComponent *aac);
};

#endif

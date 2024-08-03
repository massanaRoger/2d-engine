#ifndef AABB_H
#define AABB_H
#include "glm/vec2.hpp"
#include "Object.h"
#include "components/Components.h"

struct AABB {
    BoxComponent* aabbc;
    MassComponent* mc;

    AABB(BoxComponent* aabbc, MassComponent* mass);
};



#endif

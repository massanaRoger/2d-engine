#ifndef AABB_H
#define AABB_H
#include "glm/vec2.hpp"
#include "Object.h"
#include "components/Components.h"

struct AABB {
    AABBComponent* aabbc;
    MassComponent* mc;

    AABB(AABBComponent* aabbc, MassComponent* mass);
};



#endif

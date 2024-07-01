#ifndef PHYSICSENGINE_H
#define PHYSICSENGINE_H
#include <vector>

#include "../Circle.h"

class PhysicsEngine {
public:
    static void update(std::vector<Circle>* objects, float deltaTime);
};

#endif
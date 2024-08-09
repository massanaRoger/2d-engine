#include "Renderer.h"

#include <iostream>

#include <glad/glad.h>

#define GLM_ENABLE_EXPERIMENTAL
#include "Scene.h"
#include "SceneView.h"
#include "utils.h"
#include "components/Components.h"
#include "glm/gtx/string_cast.hpp"
#include "shader/Shader.h"
#include "physics/PhysicsEngine.h"
#include "physics/Transformations.h"

void Renderer::draw(Shader &shader) {
    shader.use();

    float vertices[] = {
        1.0f, 1.0f, 0.0f,  // top right
        1.0f, -1.0f, 0.0f,  // bottom right
        -1.0f, -1.0f, 0.0f,  // bottom left
        -1.0f, 1.0f, 0.0f,   // top left
   };

    unsigned int indices[] = {  // note that we start from 0!
        0, 1, 3,   // first triangle
        1, 2, 3    // second triangle
    };

    for (EntityID ent : SceneView<CenterOfMassComponent, CircleComponent, TransformComponent, ColorComponent>(&m_scene)) {
        glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_EBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

        auto centerOfMassComponent = m_scene.Get<CenterOfMassComponent>(ent);
        auto circleComponent = m_scene.Get<CircleComponent>(ent);
        auto transformComponent = m_scene.Get<TransformComponent>(ent);
        auto colorComponent = m_scene.Get<ColorComponent>(ent);

        shader.setMat4("transform", transformComponent->transformMatrix);
        shader.setMat4("u_projection", m_projection);
        shader.setVec2("u_center", centerOfMassComponent->centerOfMass.x, centerOfMassComponent->centerOfMass.y);
        shader.setVec3("u_color", colorComponent->color);
        shader.setFloat("u_radius", circleComponent->radius);
        shader.setInt("u_objType", 0);

        glBindVertexArray(m_VAO);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);
    }

    for (EntityID ent : SceneView<BoxComponent, TransformComponent, ColorComponent>(&m_scene)) {
        auto boxComponent = m_scene.Get<BoxComponent>(ent);
        auto transformComponent = m_scene.Get<TransformComponent>(ent);
        auto colorComponent = m_scene.Get<ColorComponent>(ent);

        glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
        glBufferData(GL_ARRAY_BUFFER, boxComponent->vertices.size() * sizeof(glm::vec3), boxComponent->vertices.data(), GL_STATIC_DRAW);

        shader.setMat4("transform", transformComponent->transformMatrix);
        shader.setMat4("u_projection", m_projection);
        shader.setInt("u_objType", 1);
        shader.setVec3("u_color", colorComponent->color);

        glBindVertexArray(m_VAO);
        glDrawArrays(GL_TRIANGLE_FAN, 0, boxComponent->vertices.size());
    }

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

void Renderer::update(float deltaTime) {
    float damping = 0.8f;

    for (EntityID ent : SceneView<VelocityComponent, AccelerationComponent, CenterOfMassComponent,
        AngularAccelerationComponent, InertiaComponent, OrientationComponent, TransformComponent, MovingComponent>(&m_scene)) {

        auto centerOfMassComponent = m_scene.Get<CenterOfMassComponent>(ent);
        auto velocityComponent = m_scene.Get<VelocityComponent>(ent);
        auto accelerationComponent = m_scene.Get<AccelerationComponent>(ent);
        auto transformComponent = m_scene.Get<TransformComponent>(ent);
        auto orientationComponent = m_scene.Get<OrientationComponent>(ent);
        auto angularVelocityComponent = m_scene.Get<AngularVelocityComponent>(ent);
        auto angularAccelerationComponent = m_scene.Get<AngularAccelerationComponent>(ent);

        centerOfMassComponent->centerOfMass += velocityComponent->velocity * deltaTime;
        orientationComponent->orientation += angularVelocityComponent->angularVelocity * deltaTime;

        float dampingDelta = std::pow(damping, deltaTime);

        velocityComponent->velocity =
                velocityComponent->velocity * dampingDelta +
                accelerationComponent->acceleration * deltaTime;

        angularVelocityComponent->angularVelocity = angularVelocityComponent->angularVelocity * dampingDelta + angularAccelerationComponent->angularAcceleration * deltaTime;

        Transformations::updateMatrix(transformComponent->transformMatrix, centerOfMassComponent->centerOfMass, orientationComponent->orientation);
    }

    // Check collision circle box
    for (EntityID cEntity : SceneView<VelocityComponent, CircleComponent, MassComponent, AngularVelocityComponent, InertiaComponent, CenterOfMassComponent, FrictionComponent>(&m_scene)) {
        auto circleComp = m_scene.Get<CircleComponent>(cEntity);
        auto cPos = m_scene.Get<CenterOfMassComponent>(cEntity);
        auto cVel = m_scene.Get<VelocityComponent>(cEntity);
        auto cMass = m_scene.Get<MassComponent>(cEntity);
        auto cAngVel = m_scene.Get<AngularVelocityComponent>(cEntity);
        auto cInvInertia = m_scene.Get<InertiaComponent>(cEntity);
        auto cFriction = m_scene.Get<FrictionComponent>(cEntity);

        for (EntityID boxEntity : SceneView<BoxComponent, MassComponent, VelocityComponent, CenterOfMassComponent, TransformComponent, AngularVelocityComponent, InertiaComponent, FrictionComponent>(&m_scene)) {
            // Can't be a boxEntity and circleEntity at the same time
            assert(boxEntity != cEntity);

            auto boxComp = m_scene.Get<BoxComponent>(boxEntity);
            auto transfComp = m_scene.Get<TransformComponent>(boxEntity);
            auto boxMass = m_scene.Get<MassComponent>(boxEntity);
            auto boxCenter = m_scene.Get<CenterOfMassComponent>(boxEntity);
            auto boxVelocity = m_scene.Get<VelocityComponent>(boxEntity);
            auto boxAngularVelocity = m_scene.Get<AngularVelocityComponent>(boxEntity);
            auto boxInverseInertia = m_scene.Get<InertiaComponent>(boxEntity);
            auto boxFriction = m_scene.Get<FrictionComponent>(boxEntity);

            Manifold m{};
            std::vector<glm::vec3> boxVertices = Transformations::getWorldVertices(boxComp->vertices, transfComp->transformMatrix);
            if (m.CirclevsBox(cPos->centerOfMass, circleComp->radius, boxVertices, boxCenter->centerOfMass)) {
                m.ApplyPositionalCorrection(cPos->centerOfMass, boxCenter->centerOfMass, cMass->inverseMass, boxMass->inverseMass);
                PhysicsEngine::resolveRotationalCollisionWithFriction(m, cPos->centerOfMass, cVel->velocity, cAngVel->angularVelocity, cInvInertia->invInertia,
               cMass->inverseMass, cFriction->staticFriction, cFriction->dynamicFriction, boxCenter->centerOfMass, boxVelocity->velocity, boxAngularVelocity->angularVelocity,
               boxMass->inverseMass, boxInverseInertia->invInertia, boxFriction->staticFriction, boxFriction->dynamicFriction);
            }
        }
    }

    // Check collisions box box
    for (EntityID e1 : SceneView<BoxComponent, MassComponent, VelocityComponent, CenterOfMassComponent, TransformComponent, AngularVelocityComponent, InertiaComponent, FrictionComponent>(&m_scene)) {
        auto boxComp1 = m_scene.Get<BoxComponent>(e1);
        auto transfComp1 = m_scene.Get<TransformComponent>(e1);
        auto boxMass1 = m_scene.Get<MassComponent>(e1);
        auto boxCenter1 = m_scene.Get<CenterOfMassComponent>(e1);
        auto boxVelocity1 = m_scene.Get<VelocityComponent>(e1);
        auto boxAngularVelocity1 = m_scene.Get<AngularVelocityComponent>(e1);
        auto boxInverseInertia1 = m_scene.Get<InertiaComponent>(e1);
        auto boxFriction1 = m_scene.Get<FrictionComponent>(e1);

        for (EntityID e2 : SceneView<BoxComponent, MassComponent, VelocityComponent, CenterOfMassComponent, TransformComponent, AngularVelocityComponent, InertiaComponent, FrictionComponent>(&m_scene)) {
            // Don't compare to itself
            if (e1 == e2) {
                continue;
            }

            auto boxComp2 = m_scene.Get<BoxComponent>(e2);
            auto transfComp2 = m_scene.Get<TransformComponent>(e2);
            auto boxMass2 = m_scene.Get<MassComponent>(e2);
            auto boxCenter2 = m_scene.Get<CenterOfMassComponent>(e2);
            auto boxVelocity2 = m_scene.Get<VelocityComponent>(e2);
            auto boxAngularVelocity2 = m_scene.Get<AngularVelocityComponent>(e2);
            auto boxInverseInertia2 = m_scene.Get<InertiaComponent>(e2);
            auto boxFriction2 = m_scene.Get<FrictionComponent>(e2);

            Manifold m{};
            std::vector<glm::vec3> boxVertices1 = Transformations::getWorldVertices(boxComp1->vertices, transfComp1->transformMatrix);
            std::vector<glm::vec3> boxVertices2 = Transformations::getWorldVertices(boxComp2->vertices, transfComp2->transformMatrix);

            if (m.BoxvsBox(boxVertices1, boxCenter1->centerOfMass, boxVertices2, boxCenter2->centerOfMass)) {
                m.ApplyPositionalCorrection(boxCenter1->centerOfMass, boxCenter2->centerOfMass, boxMass1->inverseMass, boxMass2->inverseMass);

                PhysicsEngine::resolveRotationalCollisionWithFriction(m, boxCenter1->centerOfMass, boxVelocity1->velocity, boxAngularVelocity1->angularVelocity, boxInverseInertia1->invInertia,
                   boxMass1->inverseMass, boxFriction1->staticFriction, boxFriction1->dynamicFriction, boxCenter2->centerOfMass, boxVelocity2->velocity, boxAngularVelocity2->angularVelocity,
                   boxMass2->inverseMass, boxInverseInertia2->invInertia, boxFriction2->staticFriction, boxFriction2->dynamicFriction);
            }
        }
    }

    // Check for collision with circle circle
    for (EntityID e1 : SceneView<CenterOfMassComponent, VelocityComponent, CircleComponent, MassComponent, AngularVelocityComponent, InertiaComponent, FrictionComponent>(&m_scene)) {
        auto circleComp1 = m_scene.Get<CircleComponent>(e1);
        auto cPos1 = m_scene.Get<CenterOfMassComponent>(e1);
        auto cVel1 = m_scene.Get<VelocityComponent>(e1);
        auto cMass1 = m_scene.Get<MassComponent>(e1);
        auto cAng1 = m_scene.Get<AngularVelocityComponent>(e1);
        auto cInertia1 = m_scene.Get<InertiaComponent>(e1);
        auto cFriction1 = m_scene.Get<FrictionComponent>(e1);

        for (EntityID e2 : SceneView<CenterOfMassComponent, VelocityComponent, CircleComponent, MassComponent, AngularVelocityComponent, InertiaComponent, FrictionComponent>(&m_scene)) {
            // If we are testing the same circle we do nothing
            if (e1 == e2) {
                continue;
            }
            auto circleComp2 = m_scene.Get<CircleComponent>(e2);
            auto cPos2 = m_scene.Get<CenterOfMassComponent>(e2);
            auto cVel2 = m_scene.Get<VelocityComponent>(e2);
            auto cMass2 = m_scene.Get<MassComponent>(e2);
            auto cAng2 = m_scene.Get<AngularVelocityComponent>(e2);
            auto cInertia2 = m_scene.Get<InertiaComponent>(e2);
            auto cFriction2 = m_scene.Get<FrictionComponent>(e2);

            Manifold m{};

            if (m.CirclevsCircle(cPos1->centerOfMass, circleComp1->radius, cPos2->centerOfMass, circleComp2->radius)) {
                m.ApplyPositionalCorrection(cPos1->centerOfMass, cPos2->centerOfMass, cMass1->inverseMass, cMass2->inverseMass);
                PhysicsEngine::resolveRotationalCollisionWithFriction(m, cPos1->centerOfMass, cVel1->velocity, cAng1->angularVelocity, cInertia1->invInertia,
                   cMass1->inverseMass, cFriction1->staticFriction, cFriction1->dynamicFriction, cPos2->centerOfMass, cVel2->velocity, cAng2->angularVelocity,
                   cMass2->inverseMass, cInertia2->invInertia, cFriction2->staticFriction, cFriction2->dynamicFriction);
            }
        }
    }

}

Renderer::~Renderer() {
    glDeleteVertexArrays(1, &m_VAO);
    glDeleteBuffers(1, &m_VBO);
    glDeleteBuffers(1, &m_EBO);
}

Renderer::Renderer(): m_VAO(-1), m_VBO(-1), m_EBO(-1), m_scene() {
    glGenVertexArrays(1, &m_VAO);
    glBindVertexArray(m_VAO);

    glGenBuffers(1, &m_VBO);
    glBindBuffer(GL_ARRAY_BUFFER, m_VBO);

    glGenBuffers(1, &m_EBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_EBO);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

void Renderer::insertCircle(float centerX, float centerY, float radius, const glm::vec3 &color) {
    EntityID circle = m_scene.NewEntity();

    // Positional and physics components
    auto centerOfMassComponent = m_scene.Assign<CenterOfMassComponent>(circle);
    auto velocityComponent = m_scene.Assign<VelocityComponent>(circle);
    auto accelerationComponent = m_scene.Assign<AccelerationComponent>(circle);
    auto massComponent = m_scene.Assign<MassComponent>(circle);
    auto circleComponent = m_scene.Assign<CircleComponent>(circle);
    auto avComponent = m_scene.Assign<AngularVelocityComponent>(circle);
    auto aaComponent = m_scene.Assign<AngularAccelerationComponent>(circle);
    auto inertiaComponent = m_scene.Assign<InertiaComponent>(circle);
    auto orientationComponent = m_scene.Assign<OrientationComponent>(circle);
    auto transformComponent = m_scene.Assign<TransformComponent>(circle);
    auto frictionComponent = m_scene.Assign<FrictionComponent>(circle);
    m_scene.Assign<MovingComponent>(circle);

    // Draw components
    auto colorComponent = m_scene.Assign<ColorComponent>(circle);

    centerOfMassComponent->centerOfMass = glm::vec3(centerX, centerY, 0.0f);
    velocityComponent->velocity = glm::vec3(0.0f, 0.0f, 0.0f);
    massComponent->inverseMass = 1.0f / 8.0f;
    accelerationComponent->acceleration = glm::vec3(0.0f, -5.0f, 0.0f);
    circleComponent->radius = radius;
    avComponent->angularVelocity = 0.0f;
    aaComponent->angularAcceleration = 0.0f;
    inertiaComponent->invInertia = 1 / (Transformations::calculateCircleInertia(1.0f / massComponent->inverseMass, radius) * 10);

    frictionComponent->staticFriction = 0.6f;
    frictionComponent->dynamicFriction = 0.4f;

    orientationComponent->orientation = 0.0f;
    transformComponent->transformMatrix = glm::mat4(1.0f);

    colorComponent->color = color;

    Transformations::updateMatrix(transformComponent->transformMatrix, centerOfMassComponent->centerOfMass, orientationComponent->orientation);
}

void Renderer::insertStaticBox(const glm::vec3& position, float width, float height, const glm::vec3 &color) {
    EntityID box = m_scene.NewEntity();
    auto boxComponent = m_scene.Assign<BoxComponent>(box);
    auto massComponent = m_scene.Assign<MassComponent>(box);
    auto centerOfMassComponent = m_scene.Assign<CenterOfMassComponent>(box);
    auto velocityComponent = m_scene.Assign<VelocityComponent>(box);
    auto accelerationComponent = m_scene.Assign<AccelerationComponent>(box);
    auto avComponent = m_scene.Assign<AngularVelocityComponent>(box);
    auto aaComponent = m_scene.Assign<AngularAccelerationComponent>(box);
    auto inertiaComponent = m_scene.Assign<InertiaComponent>(box);
    auto orientationComponent = m_scene.Assign<OrientationComponent>(box);
    auto transformComponent = m_scene.Assign<TransformComponent>(box);
    auto frictionComponent = m_scene.Assign<FrictionComponent>(box);

    // Draw components
    auto colorComponent = m_scene.Assign<ColorComponent>(box);

    boxComponent->vertices = createBoxVertices(width, height);

    centerOfMassComponent->centerOfMass = position;
    massComponent->inverseMass = 0.0f;
    transformComponent->transformMatrix = glm::mat4(1.0f);

    velocityComponent->velocity = glm::vec3(0.0f, 0.0f, 0.0f);
    accelerationComponent->acceleration = glm::vec3(0.0f, 0.0f, 0.0f);

    avComponent->angularVelocity = 0.0f;
    aaComponent->angularAcceleration = 0.0f;
    inertiaComponent->invInertia = 0.0f;

    frictionComponent->staticFriction = 0.6f;
    frictionComponent->dynamicFriction = 0.4f;

    orientationComponent->orientation = 0.0f;
    transformComponent->transformMatrix = glm::mat4(1.0f);

    colorComponent->color = color;

    Transformations::updateMatrix(transformComponent->transformMatrix, centerOfMassComponent->centerOfMass, orientationComponent->orientation);
}

void Renderer::insertBox(const glm::vec3& position, float width, float height, const glm::vec3 &color) {
    EntityID box = m_scene.NewEntity();
    auto boxComponent = m_scene.Assign<BoxComponent>(box);
    auto massComponent = m_scene.Assign<MassComponent>(box);
    auto centerOfMassComponent = m_scene.Assign<CenterOfMassComponent>(box);
    auto velocityComponent = m_scene.Assign<VelocityComponent>(box);
    auto accelerationComponent = m_scene.Assign<AccelerationComponent>(box);
    auto avComponent = m_scene.Assign<AngularVelocityComponent>(box);
    auto aaComponent = m_scene.Assign<AngularAccelerationComponent>(box);
    auto inertiaComponent = m_scene.Assign<InertiaComponent>(box);
    auto orientationComponent = m_scene.Assign<OrientationComponent>(box);
    auto transformComponent = m_scene.Assign<TransformComponent>(box);
    auto frictionComponent = m_scene.Assign<FrictionComponent>(box);

    m_scene.Assign<MovingComponent>(box);

    // Draw components
    auto colorComponent = m_scene.Assign<ColorComponent>(box);

    boxComponent->vertices = createBoxVertices(width, height);

    massComponent->inverseMass = 1.0f / 10.0f;
    centerOfMassComponent->centerOfMass = position;

    velocityComponent->velocity = glm::vec3(0.0f, 0.0f, 0.0f);
    accelerationComponent->acceleration = glm::vec3(0.0f, -5.0f, 0.0f);

    avComponent->angularVelocity = 0.0f;
    aaComponent->angularAcceleration = 0.0f;
    inertiaComponent->invInertia = 1 / (Transformations::calculateBoxInertia(1.0f / massComponent->inverseMass, width, height) * 10);

    // inertiaComponent->invInertia = 1 / Transformations::calculateBoxInertia(1.0f / massComponent->inverseMass, width, height);
    frictionComponent->staticFriction = 0.8f;
    frictionComponent->dynamicFriction = 0.6f;

    orientationComponent->orientation = 0.0f;
    transformComponent->transformMatrix = glm::mat4(1.0f);

    colorComponent->color = color;

    Transformations::updateMatrix(transformComponent->transformMatrix, centerOfMassComponent->centerOfMass, orientationComponent->orientation);
}

void Renderer::setProjection(const glm::mat4 &projection) {
    m_projection = projection;
}

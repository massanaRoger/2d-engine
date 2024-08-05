#include "Renderer.h"

#include <iostream>
#include <memory>

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

#define GLM_ENABLE_EXPERIMENTAL
#include "Scene.h"
#include "SceneView.h"
#include "utils.h"
#include "components/Components.h"
#include "glm/ext/vector_float3.hpp"
#include "glm/gtc/type_ptr.hpp"
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

    for (EntityID ent : SceneView<CenterOfMassComponent, CircleComponent, TransformComponent>(&m_scene)) {
        glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_EBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

        auto centerOfMassComponent = m_scene.Get<CenterOfMassComponent>(ent);
        auto circleComponent = m_scene.Get<CircleComponent>(ent);
        auto transformComponent = m_scene.Get<TransformComponent>(ent);

        GLint transformLoc = glGetUniformLocation(shader.programID, "transform");
        glUniformMatrix4fv(transformLoc, 1, GL_FALSE, glm::value_ptr(transformComponent->transformMatrix));

        shader.setVec2("u_center", centerOfMassComponent->centerOfMass.x, centerOfMassComponent->centerOfMass.y);
        shader.setFloat("u_radius", circleComponent->radius);
        shader.setInt("u_objType", 1);

        glBindVertexArray(m_VAO);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);
    }

    for (EntityID ent : SceneView<BoxComponent, TransformComponent>(&m_scene)) {
        auto *boxComponent = m_scene.Get<BoxComponent>(ent);
        auto *transformComponent = m_scene.Get<TransformComponent>(ent);

        glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
        glBufferData(GL_ARRAY_BUFFER, boxComponent->vertices.size() * sizeof(glm::vec3), boxComponent->vertices.data(), GL_STATIC_DRAW);

        GLint transformLoc = glGetUniformLocation(shader.programID, "transform");
        glUniformMatrix4fv(transformLoc, 1, GL_FALSE, glm::value_ptr(transformComponent->transformMatrix));

        shader.setInt("u_objType", 2);

        glBindVertexArray(m_VAO);
        glDrawArrays(GL_TRIANGLE_FAN, 0, boxComponent->vertices.size());
    }

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

void Renderer::update(float deltaTime) {
    float damping = 0.3f;

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

        float dampingDelta = std::pow(damping, deltaTime);

        velocityComponent->velocity =
                velocityComponent->velocity * dampingDelta +
                accelerationComponent->acceleration * deltaTime;

        angularVelocityComponent->angularVelocity = angularVelocityComponent->angularVelocity * dampingDelta + angularAccelerationComponent->angularAcceleration * deltaTime;

        orientationComponent->orientation += angularVelocityComponent->angularVelocity * deltaTime;
        Transformations::updateMatrix(transformComponent->transformMatrix, centerOfMassComponent->centerOfMass, orientationComponent->orientation);
    }

    // Check collision circle box
    for (EntityID cEntity : SceneView<VelocityComponent, CircleComponent, MassComponent, AngularVelocityComponent, InertiaComponent, CenterOfMassComponent>(&m_scene)) {
        auto circleComp = m_scene.Get<CircleComponent>(cEntity);
        auto cPos = m_scene.Get<CenterOfMassComponent>(cEntity);
        auto cVel = m_scene.Get<VelocityComponent>(cEntity);
        auto cMass = m_scene.Get<MassComponent>(cEntity);
        auto cAngVel = m_scene.Get<AngularVelocityComponent>(cEntity);
        auto cInvInertia = m_scene.Get<InertiaComponent>(cEntity);

        for (EntityID boxEntity : SceneView<BoxComponent, MassComponent, VelocityComponent, CenterOfMassComponent, TransformComponent, AngularVelocityComponent, InertiaComponent>(&m_scene)) {
            // Can't be a boxEntity and circleEntity at the same time
            assert(boxEntity != cEntity);

            auto boxComp = m_scene.Get<BoxComponent>(boxEntity);
            auto transfComp = m_scene.Get<TransformComponent>(boxEntity);
            auto boxMass = m_scene.Get<MassComponent>(boxEntity);
            auto boxCenter = m_scene.Get<CenterOfMassComponent>(boxEntity);
            auto boxVelocity = m_scene.Get<VelocityComponent>(boxEntity);
            auto boxAngularVelocity = m_scene.Get<AngularVelocityComponent>(boxEntity);
            auto boxInverseInertia = m_scene.Get<InertiaComponent>(boxEntity);

            Manifold m{};
            std::vector<glm::vec3> boxVertices = Transformations::getWorldVertices(boxComp->vertices, transfComp->transformMatrix);
            if (m.CirclevsBox(cPos->centerOfMass, circleComp->radius, boxVertices, boxCenter->centerOfMass)) {
                m.ApplyPositionalCorrection(cPos->centerOfMass, boxCenter->centerOfMass, cMass->inverseMass, boxMass->inverseMass);
                PhysicsEngine::resolveRotationalCollision(m,  cPos->centerOfMass, cVel->velocity, cAngVel->angularVelocity, cMass->inverseMass, cInvInertia->invInertia, boxCenter->centerOfMass, boxVelocity->velocity, boxAngularVelocity->angularVelocity, boxInverseInertia->invInertia,
                    boxMass->inverseMass);
            }
        }
    }

    // Check collisions box box
    for (EntityID e1 : SceneView<BoxComponent, MassComponent, VelocityComponent, CenterOfMassComponent, TransformComponent, AngularVelocityComponent, InertiaComponent>(&m_scene)) {
        auto boxComp1 = m_scene.Get<BoxComponent>(e1);
        auto transfComp1 = m_scene.Get<TransformComponent>(e1);
        auto boxMass1 = m_scene.Get<MassComponent>(e1);
        auto boxCenter1 = m_scene.Get<CenterOfMassComponent>(e1);
        auto boxVelocity1 = m_scene.Get<VelocityComponent>(e1);
        auto boxAngularVelocity1 = m_scene.Get<AngularVelocityComponent>(e1);
        auto boxInverseInertia1 = m_scene.Get<InertiaComponent>(e1);

        for (EntityID e2 : SceneView<BoxComponent, MassComponent, VelocityComponent, CenterOfMassComponent, TransformComponent, AngularVelocityComponent, InertiaComponent>(&m_scene)) {
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

            Manifold m{};
            std::vector<glm::vec3> boxVertices1 = Transformations::getWorldVertices(boxComp1->vertices, transfComp1->transformMatrix);
            std::vector<glm::vec3> boxVertices2 = Transformations::getWorldVertices(boxComp2->vertices, transfComp2->transformMatrix);

            if (m.BoxvsBox(boxVertices1, boxCenter1->centerOfMass, boxVertices2, boxCenter2->centerOfMass)) {
                m.ApplyPositionalCorrection(boxCenter1->centerOfMass, boxCenter2->centerOfMass, boxMass1->inverseMass, boxMass2->inverseMass);
                PhysicsEngine::resolveRotationalCollision(m, boxCenter1->centerOfMass, boxVelocity1->velocity, boxAngularVelocity1->angularVelocity, boxInverseInertia1->invInertia,
                    boxMass1->inverseMass, boxCenter2->centerOfMass, boxVelocity2->velocity, boxAngularVelocity2->angularVelocity, boxMass2->inverseMass, boxInverseInertia2->invInertia);
            }
        }
    }

    // Check for collision with circle circle
    for (EntityID e1 : SceneView<CenterOfMassComponent, VelocityComponent, CircleComponent, MassComponent, AngularVelocityComponent, InertiaComponent>(&m_scene)) {
        auto circleComp1 = m_scene.Get<CircleComponent>(e1);
        auto cPos1 = m_scene.Get<CenterOfMassComponent>(e1);
        auto cVel1 = m_scene.Get<VelocityComponent>(e1);
        auto cMass1 = m_scene.Get<MassComponent>(e1);
        auto cAng1 = m_scene.Get<AngularVelocityComponent>(e1);
        auto cInertia1 = m_scene.Get<InertiaComponent>(e1);

        for (EntityID e2 : SceneView<CenterOfMassComponent, VelocityComponent, CircleComponent, MassComponent, AngularVelocityComponent, InertiaComponent>(&m_scene)) {
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

            Manifold m{};

            if (m.CirclevsCircle(cPos1->centerOfMass, circleComp1->radius, cPos2->centerOfMass, circleComp2->radius)) {
                m.ApplyPositionalCorrection(cPos1->centerOfMass, cPos2->centerOfMass, cMass1->inverseMass, cMass2->inverseMass);
                PhysicsEngine::resolveRotationalCollision(m, cPos1->centerOfMass, cVel1->velocity, cAng1->angularVelocity, cInertia1->invInertia,
                    cMass1->inverseMass, cPos2->centerOfMass, cVel2->velocity, cAng2->angularVelocity, cMass2->inverseMass, cInertia2->invInertia);
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

void Renderer::insertCircle(float centerX, float centerY, float radius) {
    EntityID circle = m_scene.NewEntity();
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
    m_scene.Assign<MovingComponent>(circle);

    centerOfMassComponent->centerOfMass = glm::vec3(centerX, centerY, 0.0f);
    velocityComponent->velocity = glm::vec3(0.0f, 0.0f, 0.0f);
    massComponent->inverseMass = 1.0f;
    accelerationComponent->acceleration = glm::vec3(0.0f, -5.0f, 0.0f);
    circleComponent->radius = radius;
    avComponent->angularVelocity = 0.0f;
    aaComponent->angularAcceleration = 0.0f;
    inertiaComponent->invInertia = 1.0f;

    orientationComponent->orientation = 0.0f;
    transformComponent->transformMatrix = glm::mat4(1.0f);

    Transformations::updateMatrix(transformComponent->transformMatrix, centerOfMassComponent->centerOfMass, orientationComponent->orientation);
}

void Renderer::insertStaticBox(const glm::vec3& position, float width, float height) {
    EntityID box = m_scene.NewEntity();
    auto *boxComponent = m_scene.Assign<BoxComponent>(box);
    auto *massComponent = m_scene.Assign<MassComponent>(box);
    auto *centerOfMassComponent = m_scene.Assign<CenterOfMassComponent>(box);
    auto *velocityComponent = m_scene.Assign<VelocityComponent>(box);
    auto *accelerationComponent = m_scene.Assign<AccelerationComponent>(box);
    auto *avComponent = m_scene.Assign<AngularVelocityComponent>(box);
    auto *aaComponent = m_scene.Assign<AngularAccelerationComponent>(box);
    auto *inertiaComponent = m_scene.Assign<InertiaComponent>(box);
    auto *orientationComponent = m_scene.Assign<OrientationComponent>(box);
    auto *transformComponent = m_scene.Assign<TransformComponent>(box);

    boxComponent->vertices = createBoxVertices(width, height);

    centerOfMassComponent->centerOfMass = position;
    massComponent->inverseMass = 0.0f;
    transformComponent->transformMatrix = glm::mat4(1.0f);

    velocityComponent->velocity = glm::vec3(0.0f, 0.0f, 0.0f);
    accelerationComponent->acceleration = glm::vec3(0.0f, 0.0f, 0.0f);

    avComponent->angularVelocity = 0.0f;
    aaComponent->angularAcceleration = 0.0f;
    inertiaComponent->invInertia = 0.0f;

    orientationComponent->orientation = 0.0f;
    transformComponent->transformMatrix = glm::mat4(1.0f);

    Transformations::updateMatrix(transformComponent->transformMatrix, centerOfMassComponent->centerOfMass, orientationComponent->orientation);
}

void Renderer::insertBox(const glm::vec3& position, float width, float height) {
    EntityID box = m_scene.NewEntity();
    auto *boxComponent = m_scene.Assign<BoxComponent>(box);
    auto *massComponent = m_scene.Assign<MassComponent>(box);
    auto *centerOfMassComponent = m_scene.Assign<CenterOfMassComponent>(box);
    auto *velocityComponent = m_scene.Assign<VelocityComponent>(box);
    auto *accelerationComponent = m_scene.Assign<AccelerationComponent>(box);
    auto *avComponent = m_scene.Assign<AngularVelocityComponent>(box);
    auto *aaComponent = m_scene.Assign<AngularAccelerationComponent>(box);
    auto *inertiaComponent = m_scene.Assign<InertiaComponent>(box);
    auto *orientationComponent = m_scene.Assign<OrientationComponent>(box);
    auto *transformComponent = m_scene.Assign<TransformComponent>(box);
    m_scene.Assign<MovingComponent>(box);

    boxComponent->vertices = createBoxVertices(width, height);

    massComponent->inverseMass = 1.0f;
    centerOfMassComponent->centerOfMass = position;

    velocityComponent->velocity = glm::vec3(0.0f, 0.0f, 0.0f);
    accelerationComponent->acceleration = glm::vec3(0.0f, -5.0f, 0.0f);

    avComponent->angularVelocity = 0.0f;
    aaComponent->angularAcceleration = 0.0f;
    inertiaComponent->invInertia = 1.0f;

    // inertiaComponent->inertia = PhysicsEngine::calculateMomentOfInertia(min, max, 1.0f / massComponent->inverseMass);
    orientationComponent->orientation = 0.0f;
    transformComponent->transformMatrix = glm::mat4(1.0f);
    Transformations::updateMatrix(transformComponent->transformMatrix, centerOfMassComponent->centerOfMass, orientationComponent->orientation);
}


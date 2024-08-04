#include "Renderer.h"

#include <iostream>
#include <memory>

#include "AABB.h"
#include "Circle.h"
#include "Polygon.h"

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

#define GLM_ENABLE_EXPERIMENTAL
#include "Object.h"
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
/*
    for (EntityID ent : SceneView<PolygonComponent>(&m_scene)) {
        auto *polygonComponent = m_scene.Get<PolygonComponent>(ent);

        glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
        glBufferData(GL_ARRAY_BUFFER, polygonComponent->vertices.size() * sizeof(glm::vec3), polygonComponent->vertices.data(), GL_STATIC_DRAW);

        auto transform = glm::mat4(1.0f);

        transform = glm::rotate(transform, glm::radians(polygonComponent->rotation), glm::vec3(0.0, 0.0, 1.0));

        GLint transformLoc = glGetUniformLocation(shader.programID, "transform");
        glUniformMatrix4fv(transformLoc, 1, GL_FALSE, &transform[0][0]);

        shader.setInt("u_objType", 2);

        glBindVertexArray(m_VAO);
        glDrawArrays(GL_TRIANGLE_FAN, 0, polygonComponent->vertices.size());
    }*/

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

void Renderer::update(float deltaTime) {
    float damping = 0.5f;

    // Update polygons
    /*for (EntityID ent : SceneView<PolygonComponent, VelocityComponent, AccelerationComponent>(&m_scene)) {
        auto *polygonComponent = m_scene.Get<PolygonComponent>(ent);
        auto velocityComponent = m_scene.Get<VelocityComponent>(ent);
        auto accelerationComponent = m_scene.Get<AccelerationComponent>(ent);

        for (auto &v : polygonComponent->vertices) {
            v = v + velocityComponent->velocity * deltaTime;
        }

        velocityComponent->velocity = velocityComponent->velocity * std::pow(damping, deltaTime) + accelerationComponent->acceleration * deltaTime;
    }*/

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

        velocityComponent->velocity =
                velocityComponent->velocity * std::pow(damping, deltaTime) +
                accelerationComponent->acceleration * deltaTime;

        angularVelocityComponent->angularVelocity += angularAccelerationComponent->angularAcceleration * deltaTime;

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
                PhysicsEngine::resolveCollisionBoxCircle(m, boxCenter->centerOfMass, boxVelocity->velocity, boxAngularVelocity->angularVelocity, boxInverseInertia->invInertia,
                    boxMass->inverseMass, cPos->centerOfMass, cVel->velocity, cAngVel->angularVelocity, cMass->inverseMass, cInvInertia->invInertia);
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
                PhysicsEngine::resolveCollisionBoxCircle(m, cPos1->centerOfMass, cVel1->velocity, cAng1->angularVelocity, cInertia1->invInertia,
                    cMass1->inverseMass, cPos2->centerOfMass, cVel2->velocity, cAng2->angularVelocity, cMass2->inverseMass, cInertia2->invInertia);
            }
        }
    }

/*
    // Check collision circle polygon
    for (EntityID cEntity : SceneView<PositionComponent, VelocityComponent, AccelerationComponent, CircleComponent, MassComponent>(&m_scene)) {
        auto circleComp = m_scene.Get<CircleComponent>(cEntity);
        auto cPos = m_scene.Get<PositionComponent>(cEntity);
        auto cVel = m_scene.Get<VelocityComponent>(cEntity);
        auto cAcc = m_scene.Get<AccelerationComponent>(cEntity);
        auto cMass = m_scene.Get<MassComponent>(cEntity);

        Circle circle(cPos, cVel, cAcc, cMass, circleComp);

        for (EntityID pEntity : SceneView<PolygonComponent, VelocityComponent, AccelerationComponent, MassComponent, AngularVelocityComponent, AngularAccelerationComponent>(&m_scene)) {
            auto polygonComp = m_scene.Get<PolygonComponent>(pEntity);
            auto pVel = m_scene.Get<VelocityComponent>(pEntity);
            auto pAcc = m_scene.Get<AccelerationComponent>(pEntity);
            auto pMass = m_scene.Get<MassComponent>(pEntity);
            auto pAngVel = m_scene.Get<AngularVelocityComponent>(pEntity);
            auto pAngAcc = m_scene.Get<AngularAccelerationComponent>(pEntity);

            Polygon polygon(pVel, pAcc, pMass, polygonComp, pAngVel, pAngAcc);

            if (PhysicsEngine::checkCollisionPolygonAABB(polygon, circle)) {
                Manifold m{};
                m.AABBvsCircle(aabb, circle);
                PhysicsEngine::resolveCollisionAABBCircle(m, aabb, circle);
            }
        }
    }*/
}

Renderer::~Renderer() {
    glDeleteVertexArrays(1, &m_VAO);
    glDeleteBuffers(1, &m_VBO);
    glDeleteBuffers(1, &m_EBO);
    for (auto &obj : *m_objects) {
        delete obj;
    }
    delete m_objects;
}

Renderer::Renderer(): m_objects(new std::vector<Object*>()) ,m_VAO(-1), m_VBO(-1), m_EBO(-1), m_scene() {
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

#if false
void Renderer::insertAABB(float minX, float minY, float maxX, float maxY) {
    EntityID aabb = m_scene.NewEntity();
    auto *boxComponent = m_scene.Assign<BoxComponent>(aabb);
    auto *massComponent = m_scene.Assign<MassComponent>(aabb);
    boxComponent->min = glm::vec3(minX, minY, 0.0f);
    boxComponent->max = glm::vec3(maxX, maxY, 0.0f);
    massComponent->inverseMass = 1.0f;
}
#endif

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
    inertiaComponent->invInertia = 10.0f;

    // inertiaComponent->inertia = PhysicsEngine::calculateMomentOfInertia(min, max, 1.0f / massComponent->inverseMass);
    orientationComponent->orientation = 0.0f;
    transformComponent->transformMatrix = glm::mat4(1.0f);
    Transformations::updateMatrix(transformComponent->transformMatrix, centerOfMassComponent->centerOfMass, orientationComponent->orientation);

    std::cout << "COM: " << centerOfMassComponent->centerOfMass.x << " " << centerOfMassComponent->centerOfMass.y << std::endl;
}


void Renderer::insertPolygon(std::vector<glm::vec3>&& vertices) {
    EntityID polygon = m_scene.NewEntity();
    auto *polygonComponent = m_scene.Assign<PolygonComponent>(polygon);
    auto *avComponent = m_scene.Assign<AngularVelocityComponent>(polygon);
    auto *aaComponent = m_scene.Assign<AngularAccelerationComponent>(polygon);
    auto *accComponent = m_scene.Assign<AccelerationComponent>(polygon);
    auto *velComponent = m_scene.Assign<VelocityComponent>(polygon);
    auto *massComponent = m_scene.Assign<MassComponent>(polygon);
    auto *inertiaComponent = m_scene.Assign<InertiaComponent>(polygon);

    polygonComponent->vertices = vertices;
    polygonComponent->rotation = 0;

    velComponent->velocity = glm::vec3(0.0f, 0.0f, 0.0f);
    accComponent->acceleration = glm::vec3(0.0f, -5.0f, 0.0f);
    massComponent->inverseMass = 1.0f;
    inertiaComponent->invInertia = Polygon::calculateRotationalInertia(vertices, massComponent->inverseMass);

    avComponent->angularVelocity = 0.0f;
    aaComponent->angularAcceleration = 0.0f;
}

std::vector<Object*>* Renderer::objects() const {
    return m_objects;
}

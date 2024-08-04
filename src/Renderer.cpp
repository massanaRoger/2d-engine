#include "Renderer.h"

#include <iostream>
#include <memory>

#include "AABB.h"
#include "Circle.h"
#include "Polygon.h"

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

#include "Object.h"
#include "Scene.h"
#include "SceneView.h"
#include "utils.h"
#include "components/Components.h"
#include "glm/ext/matrix_transform.hpp"
#include "glm/ext/vector_float3.hpp"
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

    for (EntityID ent : SceneView<PositionComponent, CircleComponent>(&m_scene)) {
        glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_EBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

        auto* positionComponent = m_scene.Get<PositionComponent>(ent);
        auto* circleComponent = m_scene.Get<CircleComponent>(ent);

        auto transformMatrix = glm::mat4(1.0f);
        transformMatrix = glm::translate(transformMatrix, positionComponent->position);

        GLint transformLoc = glGetUniformLocation(shader.programID, "transform");
        glUniformMatrix4fv(transformLoc, 1, GL_FALSE, &transformMatrix[0][0]);

        shader.setVec2("u_center", positionComponent->position.x, positionComponent->position.y);
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
        glUniformMatrix4fv(transformLoc, 1, GL_FALSE, &transformComponent->transformMatrix[0][0]);

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
    // Update Circles
    for (EntityID ent :
            SceneView<PositionComponent, VelocityComponent, AccelerationComponent,
                    CircleComponent>(&m_scene)) {
        auto positionComponent = m_scene.Get<PositionComponent>(ent);
        auto velocityComponent = m_scene.Get<VelocityComponent>(ent);
        auto accelerationComponent = m_scene.Get<AccelerationComponent>(ent);

        positionComponent->position =
                positionComponent->position + velocityComponent->velocity * deltaTime;
        velocityComponent->velocity =
                velocityComponent->velocity * std::pow(damping, deltaTime) +
                accelerationComponent->acceleration * deltaTime;
    }

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

    for (EntityID ent : SceneView<BoxComponent, VelocityComponent, AccelerationComponent, CenterOfMassComponent,
        AngularAccelerationComponent, AngularAccelerationComponent, InertiaComponent, OrientationComponent, TransformComponent>(&m_scene)) {

        auto centerOfMassComponent = m_scene.Get<CenterOfMassComponent>(ent);

        auto velocityComponent = m_scene.Get<VelocityComponent>(ent);
        auto accelerationComponent = m_scene.Get<AccelerationComponent>(ent);
        auto transformComponent = m_scene.Get<TransformComponent>(ent);
        auto orientationComponent = m_scene.Get<OrientationComponent>(ent);

        centerOfMassComponent->centerOfMass += velocityComponent->velocity * deltaTime;

        velocityComponent->velocity =
                velocityComponent->velocity * std::pow(damping, deltaTime) +
                accelerationComponent->acceleration * deltaTime;

        Transformations::updateMatrix(transformComponent->transformMatrix, centerOfMassComponent->centerOfMass, orientationComponent->orientation);
    }

    // Check collision circle box
    for (EntityID cEntity : SceneView<PositionComponent, VelocityComponent, AccelerationComponent, CircleComponent, MassComponent>(&m_scene)) {
        auto circleComp = m_scene.Get<CircleComponent>(cEntity);
        auto cPos = m_scene.Get<PositionComponent>(cEntity);
        auto cVel = m_scene.Get<VelocityComponent>(cEntity);
        auto cAcc = m_scene.Get<AccelerationComponent>(cEntity);
        auto cMass = m_scene.Get<MassComponent>(cEntity);

        for (EntityID boxEntity : SceneView<BoxComponent, MassComponent, CenterOfMassComponent, TransformComponent>(&m_scene)) {
            // Can't be a boxEntity and circleEntity at the same time
            assert(boxEntity != cEntity);

            auto boxComp = m_scene.Get<BoxComponent>(boxEntity);
            auto transfComp = m_scene.Get<TransformComponent>(boxEntity);
            auto boxMass = m_scene.Get<MassComponent>(boxEntity);
            auto boxCenter = m_scene.Get<CenterOfMassComponent>(boxEntity);

            Manifold m{};
            std::vector<glm::vec3> boxVertices = Transformations::getWorldVertices(boxComp->vertices, transfComp->transformMatrix);
            if (m.CirclevsBox(cPos->position, circleComp->radius, boxVertices, boxCenter->centerOfMass)) {
                PhysicsEngine::resolveCollisionAABBCircle(boxVertices, cPos->position, cVel->velocity, cMass->inverseMass, circleComp->radius);
            }
        }
    }

    // Check for collision with circle circle
    for (EntityID e1 : SceneView<PositionComponent, VelocityComponent, AccelerationComponent, CircleComponent, MassComponent>(&m_scene)) {
        auto circleComp1 = m_scene.Get<CircleComponent>(e1);
        auto cPos1 = m_scene.Get<PositionComponent>(e1);
        auto cVel1 = m_scene.Get<VelocityComponent>(e1);
        auto cAcc1 = m_scene.Get<AccelerationComponent>(e1);
        auto cMass1 = m_scene.Get<MassComponent>(e1);

        Circle circle1(cPos1, cVel1, cAcc1, cMass1, circleComp1);

        for (EntityID e2 : SceneView<PositionComponent, VelocityComponent, AccelerationComponent, CircleComponent, MassComponent>(&m_scene)) {
            // If we are testing the same circle we do nothing
            if (e1 == e2) {
                continue;
            }
            auto circleComp2 = m_scene.Get<CircleComponent>(e2);
            auto cPos2 = m_scene.Get<PositionComponent>(e2);
            auto cVel2 = m_scene.Get<VelocityComponent>(e2);
            auto cAcc2 = m_scene.Get<AccelerationComponent>(e2);
            auto cMass2 = m_scene.Get<MassComponent>(e2);

            Circle circle2(cPos2, cVel2, cAcc2, cMass2, circleComp2);

            if (PhysicsEngine::checkCollisionCircleCircle(circle1, circle2)) {
                PhysicsEngine::resolveCollisionCircleCircle(circle1, circle2);
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
    auto *positionComponent = m_scene.Assign<PositionComponent>(circle);
    auto *velocityComponent = m_scene.Assign<VelocityComponent>(circle);
    auto *accelerationComponent = m_scene.Assign<AccelerationComponent>(circle);
    auto *massComponent = m_scene.Assign<MassComponent>(circle);
    auto *circleComponent = m_scene.Assign<CircleComponent>(circle);

    positionComponent->position = glm::vec3(centerX, centerY, 0.0f);
    velocityComponent->velocity = glm::vec3(0.0f, 0.0f, 0.0f);
    massComponent->inverseMass = 1.0f;
    accelerationComponent->acceleration = glm::vec3(0.0f, -5.0f, 0.0f);
    circleComponent->radius = radius;
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
    auto *transformComponent = m_scene.Assign<TransformComponent>(box);

    boxComponent->vertices = createBoxVertices(width, height);

    centerOfMassComponent->centerOfMass = position;
    massComponent->inverseMass = std::numeric_limits<float>::max();
    transformComponent->transformMatrix = glm::mat4(1.0f);
    Transformations::updateMatrix(transformComponent->transformMatrix, centerOfMassComponent->centerOfMass, 0.0f);
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

    boxComponent->vertices = createBoxVertices(width, height);

    massComponent->inverseMass = 1.0f;
    centerOfMassComponent->centerOfMass = position;

    velocityComponent->velocity = glm::vec3(0.0f, 0.0f, 0.0f);
    accelerationComponent->acceleration = glm::vec3(0.0f, -5.0f, 0.0f);

    avComponent->angularVelocity = 0.0f;
    aaComponent->angularAcceleration = 0.0f;
    inertiaComponent->inertia = 1.0f;

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
    inertiaComponent->inertia = Polygon::calculateRotationalInertia(vertices, massComponent->inverseMass);

    avComponent->angularVelocity = 0.0f;
    aaComponent->angularAcceleration = 0.0f;
}

std::vector<Object*>* Renderer::objects() const {
    return m_objects;
}

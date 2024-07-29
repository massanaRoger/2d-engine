#include "Renderer.h"

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
#include "components/Components.h"
#include "glm/ext/matrix_transform.hpp"
#include "glm/ext/vector_float3.hpp"
#include "shader/Shader.h"
#include "physics/PhysicsEngine.h"

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

    for (EntityID ent : SceneView<AABBComponent>(&m_scene)) {
        glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_EBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

        auto transform = glm::mat4(1.0f);

        GLint transformLoc = glGetUniformLocation(shader.programID, "transform");
        glUniformMatrix4fv(transformLoc, 1, GL_FALSE, &transform[0][0]);

        auto *aabbComponent = m_scene.Get<AABBComponent>(ent);

        shader.setVec2("u_min", aabbComponent->min.x, aabbComponent->min.y);
        shader.setVec2("u_max", aabbComponent->max.x, aabbComponent->max.y);
        shader.setInt("u_objType", 0);

        glBindVertexArray(m_VAO);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);
    }

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

void Renderer::update(float deltaTime) {
    float damping = 0.5f;
    // Update Circles
    for (EntityID entity :
            SceneView<PositionComponent, VelocityComponent, AccelerationComponent,
                    CircleComponent>(&m_scene)) {
        auto positionComponent = m_scene.Get<PositionComponent>(entity);
        auto velocityComponent = m_scene.Get<VelocityComponent>(entity);
        auto accelerationComponent = m_scene.Get<AccelerationComponent>(entity);

        positionComponent->position =
                positionComponent->position + velocityComponent->velocity * deltaTime;
        velocityComponent->velocity =
                velocityComponent->velocity * std::pow(damping, deltaTime) +
                accelerationComponent->acceleration * deltaTime;
    }
    // Update AABB
    for (EntityID entity :
            SceneView<AABBComponent, MassComponent>(&m_scene)) {
        auto aabbComponent = m_scene.Get<AABBComponent>(entity);
        auto massComponent = m_scene.Get<MassComponent>(entity);
    }

    // Update all collisions
    for (EntityID e1 : SceneView<>(&m_scene)) {
        for (EntityID e2: SceneView<>(&m_scene)) {
            auto circleComponent1 = m_scene.Get<CircleComponent>(e1);
            auto aabbComponent1 = m_scene.Get<AABBComponent>(e1);
            auto circleComponent2 = m_scene.Get<CircleComponent>(e2);
            auto aabbComponent2 = m_scene.Get<AABBComponent>(e2);

            if (circleComponent1 && circleComponent2) {
                auto pos1 = m_scene.Get<PositionComponent>(e1);
                auto vel1 = m_scene.Get<VelocityComponent>(e1);
                auto acc1 = m_scene.Get<AccelerationComponent>(e1);
                auto mass1 = m_scene.Get<MassComponent>(e1);

                auto pos2 = m_scene.Get<PositionComponent>(e2);
                auto vel2 = m_scene.Get<VelocityComponent>(e2);
                auto acc2 = m_scene.Get<AccelerationComponent>(e2);
                auto mass2 = m_scene.Get<MassComponent>(e2);

                Circle circle1(pos1, vel1, acc1, mass1, circleComponent1);
                Circle circle2(pos2, vel2, acc2, mass2, circleComponent2);

                if (PhysicsEngine::checkCollisionCircleCircle(circle1, circle2)) {
                    PhysicsEngine::resolveCollisionCircleCircle(circle1, circle2);
                }
            }
            if (circleComponent1 && aabbComponent2) {
                auto pos1 = m_scene.Get<PositionComponent>(e1);
                auto vel1 = m_scene.Get<VelocityComponent>(e1);
                auto acc1 = m_scene.Get<AccelerationComponent>(e1);
                auto mass1 = m_scene.Get<MassComponent>(e1);

                auto aabb2 = m_scene.Get<AABBComponent>(e2);
                auto mass2 = m_scene.Get<MassComponent>(e2);
                Circle circle(pos1, vel1, acc1, mass1, circleComponent1);
                AABB aabb(aabb2, mass2);

                if (PhysicsEngine::checkCollisionAABBCircle(aabb, circle)) {
                    Manifold m{};
                    m.AABBvsCircle(aabb, circle);
                    PhysicsEngine::resolveCollisionAABBCircle(m, aabb, circle);

                }
            }
            if (aabbComponent1 && circleComponent2) {

                auto aabb1 = m_scene.Get<AABBComponent>(e1);
                auto mass1 = m_scene.Get<MassComponent>(e1);

                auto pos2 = m_scene.Get<PositionComponent>(e2);
                auto vel2 = m_scene.Get<VelocityComponent>(e2);
                auto acc2 = m_scene.Get<AccelerationComponent>(e2);
                auto mass2 = m_scene.Get<MassComponent>(e2);

                AABB aabb(aabb1, mass1);
                Circle circle(pos2, vel2, acc2, mass2, circleComponent2);

                if (PhysicsEngine::checkCollisionAABBCircle(aabb, circle)) {
                    Manifold m{};
                    m.AABBvsCircle(aabb, circle);
                    PhysicsEngine::resolveCollisionAABBCircle(m, aabb, circle);
                }
            }
        }

    }
}

/*void Renderer::draw(Shader &shader) {
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

    for (auto& object : *m_objects) {
        if (auto polygon = dynamic_cast<Polygon*>(object)) {
            glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
            glBufferData(GL_ARRAY_BUFFER, polygon->vertices.size() * sizeof(glm::vec3), polygon->vertices.data(), GL_STATIC_DRAW);

            object->draw(shader);

            glBindVertexArray(m_VAO);
            glDrawArrays(GL_TRIANGLE_FAN, 0, polygon->vertices.size());
        } else {
            glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
            glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_EBO);
            glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

            object->draw(shader);

            glBindVertexArray(m_VAO);
            glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);
        }
    }

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}*/

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


void Renderer::insertAABB(float minX, float minY, float maxX, float maxY) {
    EntityID aabb = m_scene.NewEntity();
    auto *aabbComponent = m_scene.Assign<AABBComponent>(aabb);
    auto *massComponent = m_scene.Assign<MassComponent>(aabb);
    aabbComponent->min = glm::vec3(minX, minY, 0.0f);
    aabbComponent->max = glm::vec3(maxX, maxY, 0.0f);
    massComponent->inverseMass = 1.0f;
}

void Renderer::insertPolygon(std::initializer_list<glm::vec3> il) {
    auto polygon = new Polygon(il, glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, -5.0f, 0.0f), 0.5f, 1.0f, 0.005f);
    m_objects->push_back(polygon);
}

void Renderer::insertPolygon(std::vector<glm::vec3>&& vertices) {
    auto polygon = new Polygon(std::move(vertices), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, -5.0f, 0.0f), 0.5f, 1.0f, 0.005f);
    m_objects->push_back(polygon);
}


std::vector<Object*>* Renderer::objects() const {
    return m_objects;
}

#include "Renderer.h"

#include <memory>

#include "AABB.h"
#include "Circle.h"
#include "Polygon.h"

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

#include "Object.h"
#include "glm/ext/vector_float3.hpp"
#include "shader/Shader.h"

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

Renderer::Renderer(): m_objects(new std::vector<Object*>()) ,m_VAO(-1), m_VBO(-1), m_EBO(-1) {
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

void Renderer::insertCircle(float centerX, float centerY, float radius, int numSegments) {
    auto circle = new Circle(numSegments, radius, glm::vec3(centerX, centerY, 0.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, -5.0f, 0.0f), 2.0f, 0.5f);
    m_objects->push_back(circle);
}


void Renderer::insertAABB(float minX, float minY, float maxX, float maxY) {
    auto aabb = new AABB(minX, minY, maxX, maxY);
    m_objects->push_back(aabb);
}

void Renderer::insertPolygon(std::initializer_list<glm::vec3> il) {
    auto polygon = new Polygon(il);
    m_objects->push_back(polygon);
}

void Renderer::insertPolygon(std::vector<glm::vec3>&& vertices) {
    auto polygon = new Polygon(std::move(vertices));
    m_objects->push_back(polygon);
}


std::vector<Object*>* Renderer::objects() const {
    return m_objects;
}

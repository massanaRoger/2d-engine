#include "Renderer.h"
#include "Circle.h"
#include "glm/trigonometric.hpp"

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

#include "glm/ext/matrix_transform.hpp"
#include "shader/Shader.h"

void Renderer::draw(Shader &shader) {
    shader.use();

    for (auto& circle : *m_objects) {
        float x = circle.position.x;
        float y = circle.position.y;
        float r = circle.radius;

        /*float vertices[] = {
            x+r, y+r, 0.0f,  // top right
            x+r, y-r, 0.0f,  // bottom right
            x-r, y-r, 0.0f,  // bottom left
            x-r, y+r, 0.0f,   // top left
       };*/

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

        glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_EBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

        glm::mat4 transform = glm::mat4(1.0f);
        transform = glm::translate(transform, circle.position);

        GLint transformLoc = glGetUniformLocation(shader.programID, "transform");
        glUniformMatrix4fv(transformLoc, 1, GL_FALSE, &transform[0][0]);

        shader.setVec2("u_center", circle.position.x, circle.position.y);
        shader.setFloat("u_radius", 0.9f);

        glBindVertexArray(m_VAO);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);
    }

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

Renderer::~Renderer() {
    glDeleteVertexArrays(1, &m_VAO);
    glDeleteBuffers(1, &m_VBO);
    glDeleteBuffers(1, &m_EBO);
}

Renderer::Renderer(std::vector<Circle>* objects):  m_objects(objects), m_VAO(-1), m_VBO(-1), m_EBO(-1) {
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
    Circle circle(numSegments, radius, glm::vec3(centerX, centerY, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f), 1.0f);

    m_objects->push_back(circle);
}

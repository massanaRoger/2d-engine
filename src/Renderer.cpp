#include "Renderer.h"
#include "Circle.h"
#include "glm/trigonometric.hpp"

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

#include "glm/ext/matrix_transform.hpp"
#include "shader/Shader.h"

void Renderer::draw(double descentSpeed, Shader &shader) {
    shader.use();

    glBindBuffer(GL_ARRAY_BUFFER, m_VBO);

    for (auto& circle : m_programVertices) {
        circle.updateTick(descentSpeed);

        glBufferData(GL_ARRAY_BUFFER, circle.size() * sizeof(Vertex), circle.data().data(), GL_STATIC_DRAW);

        glm::mat4 transform = glm::mat4(1.0f);
        transform = glm::translate(transform, circle.descentVector());

        GLint transformLoc = glGetUniformLocation(shader.programID, "transform");
        glUniformMatrix4fv(transformLoc, 1, GL_FALSE, &transform[0][0]);

        glBindVertexArray(m_VAO);
        glDrawArrays(GL_TRIANGLE_FAN, 0, circle.size());
    }

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}


Renderer::~Renderer() {
    glDeleteVertexArrays(1, &m_VAO);
    glDeleteBuffers(1, &m_VBO);
}

Renderer::Renderer():  m_programVertices(std::vector<Circle>()), m_VAO(-1), m_VBO(-1) {
    glGenVertexArrays(1, &m_VAO);
    glBindVertexArray(m_VAO);

    glGenBuffers(1, &m_VBO);
    glBindBuffer(GL_ARRAY_BUFFER, m_VBO);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0); // Assuming Vertex has at least 3 floats for position
    glEnableVertexAttribArray(0);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

void Renderer::insertCircle(float centerX, float centerY, float radius, int numSegments) {
    Circle circle(numSegments);
    // Center of the circle
    Vertex startVertex(centerX, centerY, 0.0f);
    circle.insert(startVertex);
    for (int i = 1; i <= numSegments; i++) {
        float theta = 2.0f * 3.1415926f * float(i - 1) / float(numSegments); // Get the current angle

        float x = radius * glm::cos(theta); // Calculate the x component
        float y = radius * glm::sin(theta); // Calculate the y component
        Vertex v(x + centerX, y + centerY, 0.0f);
        circle.insert(v);
    }

    // Close the circle by adding the first circumference vertex again
    Vertex endVertex(circle[1].x, circle[1].y, circle[1].z);
    circle.insert(endVertex);

    m_programVertices.push_back(circle);
}

#include "shader/Shader.h"
#include <array>
#include <iostream>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <vector>

#include "utils.h"
#include "glm/ext/matrix_transform.hpp"

void processInput(GLFWwindow *window);
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods);
void pixelToNDC(GLFWwindow* window, double x, double y, double* ndcX, double* ndcY);

struct Vertex {
    float x;
    float y;
    float z;

    Vertex(const float x, const float y, const float z) : x{x}, y{y}, z{z} {}
};

class Circle {
public:
    explicit Circle(std::size_t n): m_descentVector(0.0f) {
        m_vertices = std::vector<Vertex>();
        m_vertices.reserve(n);
    }

    std::vector<Vertex>& data() {
        return m_vertices;
    }

    void insert(const Vertex& vertex) {
        m_vertices.push_back(vertex);
    }

    Vertex& operator[](std::size_t idx) {
        return m_vertices[idx];
    }

    const Vertex& operator[](std::size_t idx) const {
        return m_vertices[idx];
    }

    std::size_t size() {
        return m_vertices.size();
    }

    void updateTick(double descentSpeed) {
        m_descentVector.y -= descentSpeed;
    }

    glm::vec3 &descentVector() {
        return m_descentVector;
    }

    using iterator = std::vector<Vertex>::iterator;
    using const_iterator = std::vector<Vertex>::const_iterator;

    iterator begin() {
        return m_vertices.begin();
    }

    [[nodiscard]] const_iterator begin() const {
        return m_vertices.begin();
    }

    iterator end() {
        return m_vertices.end();
    }

    [[nodiscard]] const_iterator end() const {
        return m_vertices.end();
    }
private:
    std::vector<Vertex> m_vertices;
    glm::vec3 m_descentVector;
};

unsigned int VBO, VAO;
constexpr int numSegments = 100;
constexpr int arraySegmentSize = numSegments + 2;
double deltaTime = 0.0f;
double lastFrame = 0.0f;
auto programVertices = std::vector<Circle>();

void drawCircle(float centerX, float centerY, float radius, int numSegments, Circle& circle) {
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
}


int main()
{
    if (!glfwInit())
    {
        std::cout << "Failed to initialize GLFW" << std::endl;
        return -1;
    }

    glfwWindowHint(GLFW_SAMPLES, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window;
    window = glfwCreateWindow(800, 800, "ZMMR", nullptr, nullptr);
    if (window == nullptr) {
        std::cout << "Failed to open GLFW window" << std::endl;
        return -1;
    }
    glfwMakeContextCurrent(window);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    glViewport(0, 0, 800, 800);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetMouseButtonCallback(window, mouse_button_callback);

    Shader vertexShader = Shader(getFullPath("shaders/vertex_shader.glsl"), ShaderType::VertexShader);
    Shader fragmentShader = Shader(getFullPath("shaders/fragment_shader.glsl"), ShaderType::FragmentShader);

    unsigned int shaderProgram;
    shaderProgram = glCreateProgram();

    glAttachShader(shaderProgram, vertexShader.shaderID);
    glAttachShader(shaderProgram, fragmentShader.shaderID);
    glLinkProgram(shaderProgram);

    int  success;
    char infoLog[512];
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);

    if (!success)
    {
        glGetProgramInfoLog(shaderProgram, 512, nullptr, infoLog);
        std::cerr << "ERROR::PROGRAM::LINKING FAILED\n" << infoLog << std::endl;
    }

    glUseProgram(shaderProgram);

    glDeleteShader(vertexShader.shaderID);
    glDeleteShader(fragmentShader.shaderID);

    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    // bind the Vertex Array Object first, then bind and set vertex buffer(s), and then configure vertex attributes(s).
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
    glEnableVertexAttribArray(0);

    while(!glfwWindowShouldClose(window))
    {
        processInput(window);

        glClear(GL_COLOR_BUFFER_BIT);

        double currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;
        double descentSpeed = 1.0f * deltaTime;

        glUseProgram(shaderProgram);
        glBindVertexArray(VAO);

        for (auto& circle : programVertices) {
            circle.updateTick(descentSpeed);

            glBufferData(GL_ARRAY_BUFFER, circle.size() * sizeof(Vertex), circle.data().data(), GL_STATIC_DRAW);

            glm::mat4 transform = glm::mat4(1.0f);
            transform = glm::translate(transform, circle.descentVector());

            glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "transform"), 1, GL_FALSE, &transform[0][0]);
            glDrawArrays(GL_TRIANGLE_FAN, 0, arraySegmentSize);

        }

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
}

void processInput(GLFWwindow *window)
{

    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, true);
    }
}

void mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
        std::cout << "Clicked" << std::endl;
        double xpos, ypos;
        glfwGetCursorPos(window, &xpos, &ypos);
        double ndcX, ndcY;
        pixelToNDC(window, xpos, ypos, &ndcX, &ndcY);

        float radius = 0.05f;
        Circle circle(arraySegmentSize);
        drawCircle(ndcX, ndcY, radius, numSegments, circle);
        glBindBuffer(GL_ARRAY_BUFFER, VBO);

        programVertices.push_back(circle);
    }
}

void pixelToNDC(GLFWwindow* window, double x, double y, double* ndcX, double* ndcY) {
    int windowWidth, windowHeight;
    glfwGetWindowSize(window, &windowWidth, &windowHeight);

    *ndcX = (2.0 * x) / windowWidth - 1.0;
    *ndcY = 1.0 - (2.0 * y) / windowHeight;
}

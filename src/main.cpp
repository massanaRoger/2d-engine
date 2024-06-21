#include "shader/Shader.h"
#include <iostream>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

#include "utils.h"

void processInput(GLFWwindow *window);
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods);
void pixelToNDC(GLFWwindow* window, double x, double y, double* ndcX, double* ndcY);

void drawCircle(float centerX, float centerY, float radius, int numSegments, float vertices[][3]) {
    // Center of the circle
    vertices[0][0] = centerX;
    vertices[0][1] = centerY;
    vertices[0][2] = 0.0f;

    for (int i = 1; i <= numSegments; i++) {
        float theta = 2.0f * 3.1415926f * float(i - 1) / float(numSegments); // Get the current angle

        float x = radius * glm::cos(theta); // Calculate the x component
        float y = radius * glm::sin(theta); // Calculate the y component

        vertices[i][0] = x + centerX;
        vertices[i][1] = y + centerY;
        vertices[i][2] = 0.0f;
    }

    // Close the circle by adding the first circumference vertex again
    vertices[numSegments + 1][0] = vertices[1][0];
    vertices[numSegments + 1][1] = vertices[1][1];
    vertices[numSegments + 1][2] = vertices[1][2];
}

unsigned int numCircles = 0;
unsigned int VBO, VAO;
const int numSegments = 100;

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
    glGetProgramiv(shaderProgram, GL_COMPILE_STATUS, &success);

    if (!success)
    {
        glGetShaderInfoLog(shaderProgram, 512, nullptr, infoLog);
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

    glUseProgram(shaderProgram);

    while(!glfwWindowShouldClose(window))
    {
        processInput(window);

        glUseProgram(shaderProgram);
        glBindVertexArray(VAO);
        for (int i = 0; i < numCircles; i++) {
            glDrawArrays(GL_TRIANGLE_FAN, 0, numSegments + 2);
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

        float vertices[numSegments + 2][3];
        float radius = 0.05f;
        drawCircle(ndcX, ndcY, radius, numSegments, vertices);
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
        numCircles++;
    }
}

void pixelToNDC(GLFWwindow* window, double x, double y, double* ndcX, double* ndcY) {
    int windowWidth, windowHeight;
    glfwGetWindowSize(window, &windowWidth, &windowHeight);

    *ndcX = (2.0 * x) / windowWidth - 1.0;
    *ndcY = 1.0 - (2.0 * y) / windowHeight;
}

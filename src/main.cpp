#include "shader/Shader.h"
#include <iostream>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

#include "utils.h"

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
}


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
        std::cout << vertices[i][0] << " " << vertices[i][1] << " "  << vertices[i][2] << std::endl;
    }

    // Close the circle by adding the first circumference vertex again
    vertices[numSegments + 1][0] = vertices[1][0];
    vertices[numSegments + 1][1] = vertices[1][1];
    vertices[numSegments + 1][2] = vertices[1][2];
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
    window = glfwCreateWindow(800, 600, "ZMMR", nullptr, nullptr);
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

    // Set the circle parameters
    float centerX = 0.0f;
    float centerY = 0.0f;
    float radius = 0.5f;
    const int numSegments = 100; // The higher the number, the smoother the circle
    float vertices[numSegments + 2][3];
    drawCircle(centerX, centerY, radius, numSegments, vertices);

    unsigned int VBO, VAO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    // bind the Vertex Array Object first, then bind and set vertex buffer(s), and then configure vertex attributes(s).
    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
    glEnableVertexAttribArray(0);

    glUseProgram(shaderProgram);

    while(!glfwWindowShouldClose(window))
    {
        glUseProgram(shaderProgram);
        glBindVertexArray(VAO);
        glDrawArrays(GL_TRIANGLE_FAN, 0, numSegments + 2);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}

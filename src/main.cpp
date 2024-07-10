#include "Renderer.h"
#include "shader/Shader.h"
#include <iostream>
#include <memory>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

#include "utils.h"
#include "physics/PhysicsEngine.h"

void processInput(GLFWwindow *window);
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods);
void pixelToNDC(GLFWwindow* window, double x, double y, double* ndcX, double* ndcY);
void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);

constexpr int numSegments = 100;
constexpr int arraySegmentSize = numSegments + 2;
double deltaTime = 0.0f;
double lastFrame = 0.0f;
std::unique_ptr<Renderer> renderer;
bool isPointerCursor = false;
GLFWcursor* pointerCursor = nullptr;

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

    Shader shader = Shader(getFullPath("shaders/vertex_shader.glsl"), getFullPath("shaders/fragment_shader.glsl"));
    renderer = std::make_unique<Renderer>();
    renderer->insertAABB(-0.9, -0.9, 0.9, -0.8);
    renderer->insertPolygon({
        glm::vec3(-0.8f, 0.6f, 0.0f),
        glm::vec3(-0.3f, 0.9f, 0.0f),
        glm::vec3(0.7f, 0.0f, 0.0f),
        glm::vec3(0.5f, -0.4f, 0.0f),
        glm::vec3(0.0f, -1.0f, 0.0f)
    });

    glViewport(0, 0, 800, 800);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetMouseButtonCallback(window, mouse_button_callback);
    glfwSetKeyCallback(window, keyCallback);

    while(!glfwWindowShouldClose(window))
    {
        processInput(window);

        double currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        int width, height;
        glfwGetFramebufferSize(window, &width, &height);
        shader.setVec2("u_resolution", (float)width, (float)height);
        PhysicsEngine::update(renderer->objects(), deltaTime);

        glClear(GL_COLOR_BUFFER_BIT);

        renderer->draw(shader);

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

void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if (key == GLFW_KEY_D && action == GLFW_RELEASE) {
        if (isPointerCursor) {
            glfwSetCursor(window, nullptr);
        } else {
            if (!pointerCursor) {
                pointerCursor = glfwCreateStandardCursor(GLFW_HAND_CURSOR);
            }
            glfwSetCursor(window, pointerCursor);
        }
        isPointerCursor = !isPointerCursor;
    }
}


void mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
        double xpos, ypos;
        glfwGetCursorPos(window, &xpos, &ypos);
        double ndcX, ndcY;
        pixelToNDC(window, xpos, ypos, &ndcX, &ndcY);

        float radius = 0.05f;
        renderer->insertCircle(ndcX, ndcY, radius, numSegments);
    }
    if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS) {
        double xpos, ypos;
        glfwGetCursorPos(window, &xpos, &ypos);
        double ndcX, ndcY;
        pixelToNDC(window, xpos, ypos, &ndcX, &ndcY);
        float r = 0.05f;
        renderer->insertAABB(ndcX - r, ndcY - r, ndcX + r, ndcY + r);
    }
}

void pixelToNDC(GLFWwindow* window, double x, double y, double* ndcX, double* ndcY) {
    int windowWidth, windowHeight;
    glfwGetWindowSize(window, &windowWidth, &windowHeight);

    *ndcX = (2.0 * x) / windowWidth - 1.0;
    *ndcY = 1.0 - (2.0 * y) / windowHeight;
}

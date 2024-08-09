#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <iostream>
#include <memory>

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#include "Renderer.h"
#include "Scene.h"
#include "components/Components.h"
#include "physics/PhysicsEngine.h"
#include "shader/Shader.h"
#include "utils.h"
#include "gui/GUIManager.h"
#include "physics/Transformations.h"

void processInput(GLFWwindow *window);
void framebuffer_size_callback(GLFWwindow *window, int width, int height);
void mouse_button_callback(GLFWwindow *window, int button, int action,
                           int mods);
void pixelToNDC(GLFWwindow *window, double x, double y, double *ndcX,
                double *ndcY);
void keyCallback(GLFWwindow *window, int key, int scancode, int action,
                 int mods);

double deltaTime = 0.0f;
double lastFrame = 0.0f;
std::unique_ptr<Renderer> renderer;
std::unique_ptr<GUIManager> guiManager;
std::unique_ptr<Shader> shader;
bool isPointerCursor = false;
GLFWcursor *pointerCursor = nullptr;
std::vector<glm::vec3> polygonToInsert;
glm::mat4 projection;

int main() {
  if (!glfwInit()) {
    std::cout << "Failed to initialize GLFW" << std::endl;
    return -1;
  }

  glfwWindowHint(GLFW_SAMPLES, 4);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

  GLFWwindow *window;
  window = glfwCreateWindow(800, 800, "ZMMR", nullptr, nullptr);
  if (window == nullptr) {
    std::cout << "Failed to open GLFW window" << std::endl;
    return -1;
  }
  glfwMakeContextCurrent(window);

  if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
    std::cout << "Failed to initialize GLAD" << std::endl;
    return -1;
  }

  shader = std::make_unique<Shader>(
    getFullPath("shaders/vertex_shader.glsl"),
    getFullPath("shaders/fragment_shader.glsl")
  );
  renderer = std::make_unique<Renderer>();
  guiManager = std::make_unique<GUIManager>();
  projection = Transformations::createProjectionMatrix(800, 800);
  renderer->setProjection(projection);

  float width = 1.8f;
  float height = 0.1f;
  renderer->insertStaticBox(glm::vec3(0.0f, -0.8f, 0.0f), width, height, guiManager->GetSelectedColor());

  glViewport(0, 0, 800, 800);
  glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
  glfwSetMouseButtonCallback(window, mouse_button_callback);
  glfwSetKeyCallback(window, keyCallback);

  // Setup Dear ImGui context
  IMGUI_CHECKVERSION();
  ImGui::CreateContext();
  ImGuiIO& io = ImGui::GetIO();
  io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
  io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
  io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;         // IF using Docking Branch

  // Setup Platform/Renderer backends
  ImGui_ImplGlfw_InitForOpenGL(window, true);          // Second param install_callback=true will install GLFW callbacks and chain to existing ones.
  ImGui_ImplOpenGL3_Init();

  while (!glfwWindowShouldClose(window)) {
    glfwPollEvents();

    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
    guiManager->Render();

    processInput(window);

    double currentFrame = glfwGetTime();
    deltaTime = currentFrame - lastFrame;
    lastFrame = currentFrame;

    int width, height;
    glfwGetFramebufferSize(window, &width, &height);
    shader->setVec2("u_resolution", static_cast<float>(width), static_cast<float>(height));
    renderer->update(deltaTime);

    glClear(GL_COLOR_BUFFER_BIT);

    renderer->draw(*shader);

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    glfwSwapBuffers(window);
  }

  glfwTerminate();
  ImGui_ImplOpenGL3_Shutdown();
  ImGui_ImplGlfw_Shutdown();
  ImGui::DestroyContext();
  return 0;
}

void framebuffer_size_callback(GLFWwindow *window, int width, int height) {
  glViewport(0, 0, width, height);
  projection = Transformations::createProjectionMatrix(width, height);
  renderer->setProjection(projection);
}

void processInput(GLFWwindow *window) {
  if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
    glfwSetWindowShouldClose(window, true);
  }
}

void keyCallback(GLFWwindow *window, int key, int scancode, int action,
                 int mods) {
  if (key == GLFW_KEY_D && action == GLFW_RELEASE) {
    if (isPointerCursor) {
      if (polygonToInsert.size() > 2) {
        //renderer->insertPolygon(std::move(polygonToInsert));
        polygonToInsert.clear();
      }
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

void mouse_button_callback(GLFWwindow *window, int button, int action,
                           int mods) {
  auto& io = ImGui::GetIO();
  if (io.WantCaptureMouse || io.WantCaptureKeyboard) {
    return;
  }

  if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS &&
      !isPointerCursor) {
    double xpos, ypos;
    glfwGetCursorPos(window, &xpos, &ypos);
    double ndcX, ndcY;
    pixelToNDC(window, xpos, ypos, &ndcX, &ndcY);

    float radius = 0.05f;
    renderer->insertCircle(ndcX, ndcY, radius, guiManager->GetSelectedColor());
  }
  if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS &&
      isPointerCursor) {
    double xpos, ypos;
    glfwGetCursorPos(window, &xpos, &ypos);
    double ndcX, ndcY;
    pixelToNDC(window, xpos, ypos, &ndcX, &ndcY);

    polygonToInsert.emplace_back(ndcX, ndcY, 0.0f);
  }

  if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS) {
    double xpos, ypos;
    glfwGetCursorPos(window, &xpos, &ypos);
    double ndcX, ndcY;
    pixelToNDC(window, xpos, ypos, &ndcX, &ndcY);
    float width = 0.1f;
    float height = 0.1f;
    renderer->insertBox(glm::vec3(ndcX, ndcY, 0.0f), width, height, guiManager->GetSelectedColor());
  }
}

void pixelToNDC(GLFWwindow *window, double x, double y, double *ndcX, double *ndcY) {
  int windowWidth, windowHeight;
  glfwGetWindowSize(window, &windowWidth, &windowHeight);

  float aspectRatio = static_cast<float>(windowWidth) / static_cast<float>(windowHeight);

  // Adjust for aspect ratio
  *ndcX = ((2.0f * x) / windowWidth - 1.0f) * aspectRatio;
  *ndcY = 1.0f - (2.0f * y) / windowHeight;
}

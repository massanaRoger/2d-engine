cmake_minimum_required(VERSION 3.28)
project(2d_engine)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Set up vcpkg integration
if(NOT DEFINED CMAKE_TOOLCHAIN_FILE)
    set(CMAKE_TOOLCHAIN_FILE "${CMAKE_CURRENT_SOURCE_DIR}/vcpkg/scripts/buildsystems/vcpkg.cmake" CACHE STRING "")
endif()

# Specify the required packages
find_package(glfw3 CONFIG REQUIRED)
find_package(glad CONFIG REQUIRED)
find_package(imgui CONFIG REQUIRED)
find_package(glm CONFIG REQUIRED)

add_executable(${PROJECT_NAME}
        src/main.cpp
        src/shader/Shader.cpp
        src/utils.cpp
        src/Renderer.cpp
        src/physics/PhysicsEngine.cpp
        src/physics/Manifold.cpp
        src/Scene.cpp
        src/ComponentPool.cpp
        src/SceneView.cpp
        src/components/Components.cpp
        src/component.cpp
        src/physics/contacts.cpp
        src/physics/Transformations.cpp
        src/gui/GUIManager.cpp
)

# Define the current working directory as a macro
add_definitions(-DCURRENT_WORKING_DIR=\"${CMAKE_CURRENT_SOURCE_DIR}\")

target_link_libraries(${PROJECT_NAME} PRIVATE glfw glad::glad imgui::imgui glm::glm)

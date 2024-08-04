cmake_minimum_required(VERSION 3.28)
project(2d_engine)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

include(FetchContent)

set(GLFW_VERSION 3.4)
FetchContent_Declare(
        glfw
        URL https://github.com/glfw/glfw/archive/refs/tags/${GLFW_VERSION}.tar.gz
        FIND_PACKAGE_ARGS ${GLFW_VERSION}
)
set(BUILD_EXAMPLES OFF CACHE INTERNAL "")
FetchContent_MakeAvailable(glfw)

set(GLAD_VERSION 0.1.36)
FetchContent_Declare(
        glad
        GIT_REPOSITORY https://github.com/Dav1dde/glad.git
        GIT_TAG v${GLAD_VERSION}
)
set(GLAD_PROFILE "core"	CACHE STRING "OpenGL profile")
set(GLAD_GENERATOR "c"	CACHE STRING "Language to generate the binding for")
FetchContent_MakeAvailable(glad)

set(GLAD_VERSION 1.0.1)
FetchContent_Declare(
        glm
        GIT_REPOSITORY https://github.com/g-truc/glm.git
        GIT_TAG ${GLAD_VERSION}
)
FetchContent_MakeAvailable(glm)

# Include GLAD's include directory
include_directories(${glad_SOURCE_DIR}/include)

add_executable(${PROJECT_NAME}
        src/main.cpp
        src/shader/Shader.cpp
        src/utils.cpp
        src/Circle.cpp
        src/Renderer.cpp
        src/physics/PhysicsEngine.cpp
        src/AABB.cpp
        src/Object.cpp
        src/physics/Manifold.cpp
        src/Polygon.cpp
        src/Scene.cpp
        src/ComponentPool.cpp
        src/SceneView.cpp
        src/components/Components.cpp
        src/component.cpp
        src/Box.cpp
        src/physics/contacts.cpp
        src/physics/contacts.h
        src/physics/Transformations.cpp
        src/physics/Transformations.h
)

# Define the current working directory as a macro
add_definitions(-DCURRENT_WORKING_DIR=\"${CMAKE_CURRENT_SOURCE_DIR}\")

target_link_libraries(${PROJECT_NAME} PRIVATE glfw glad glm)


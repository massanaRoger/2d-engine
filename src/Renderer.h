#ifndef RENDERER_H
#define RENDERER_H

#include "Scene.h"
#include "shader/Shader.h"
#include "glm/glm.hpp"

class Renderer {
public:
    explicit Renderer();
    ~Renderer();
    void draw(Shader &shader);

    EntityID insertCircle(float centerX, float centerY, float radius, const glm::vec4 &color);
    EntityID insertBox(const glm::vec3& position, float width, float height, const glm::vec4 &color);
    EntityID insertStaticBox(const glm::vec3& position, float width, float height, const glm::vec4 &color);
    EntityID setHoveredCircle(const glm::vec3 &position, float radius, const glm::vec4 &color);
    void update(float deltaTime);
    void setProjection(const glm::mat4 &projection);
private:
    Scene m_scene;
    glm::mat4 m_projection;
    unsigned int m_VBO, m_VAO, m_EBO;
    EntityID m_hoveredCircle = std::numeric_limits<EntityID>::max();
};

#endif

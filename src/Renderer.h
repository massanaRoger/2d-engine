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

    void insertCircle(float centerX, float centerY, float radius);
    void insertBox(const glm::vec3& position, float width, float height);
    void insertStaticBox(const glm::vec3& position, float width, float height);
    void update(float deltaTime);
private:
    Scene m_scene;
    unsigned int m_VBO, m_VAO, m_EBO;
};

#endif

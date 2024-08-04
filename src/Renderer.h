#ifndef RENDERER_H
#define RENDERER_H

#include "Circle.h"
#include "Scene.h"
#include "shader/Shader.h"

class Renderer {
public:
    explicit Renderer();
    ~Renderer();
    void draw(Shader &shader);

    void insertCircle(float centerX, float centerY, float radius);
    void insertAABB(float minX, float minY, float maxX, float maxY);
    void insertBox(const glm::vec3& position, float width, float height);
    void insertStaticBox(const glm::vec3& position, float width, float height);
    void insertPolygon(std::initializer_list<glm::vec3> il);
    void insertPolygon(std::vector<glm::vec3>&& vertices);
    void update(float deltaTime);
    [[nodiscard]] std::vector<Object*>* objects() const;
private:
    Scene m_scene;
    unsigned int m_VBO, m_VAO, m_EBO;
    std::vector<Object*>* m_objects;
};

#endif

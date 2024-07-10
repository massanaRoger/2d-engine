#ifndef RENDERER_H
#define RENDERER_H

#include "Circle.h"
#include "shader/Shader.h"

class Renderer {
public:
    explicit Renderer();
    ~Renderer();
    void draw(Shader &shader);
    void insertCircle(float centerX, float centerY, float radius, int numSegments);
    void insertAABB(float minX, float minY, float maxX, float maxY);
    void insertPolygon(std::initializer_list<glm::vec3> il);
    void insertPolygon(std::vector<glm::vec3>&& vertices);
    [[nodiscard]] std::vector<Object*>* objects() const;
private:
    unsigned int m_VBO, m_VAO, m_EBO;
    std::vector<Object*>* m_objects;
};

#endif

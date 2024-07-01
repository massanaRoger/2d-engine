#ifndef RENDERER_H
#define RENDERER_H

#include "Circle.h"
#include "shader/Shader.h"

class Renderer {
public:
    explicit Renderer(std::vector<Circle>* objects);
    ~Renderer();
    void draw(Shader &shader);
    void insertCircle(float centerX, float centerY, float radius, int numSegments);
private:
    unsigned int m_VBO, m_VAO, m_EBO;
    std::vector<Circle>* m_objects;
};

#endif

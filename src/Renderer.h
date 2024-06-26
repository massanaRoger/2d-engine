#ifndef RENDERER_H
#define RENDERER_H

#include "Circle.h"
#include "shader/Shader.h"

class Renderer {
public:
    Renderer();
    ~Renderer();
    void draw(double descentSpeed, Shader &shader);
    void insertCircle(float centerX, float centerY, float radius, int numSegments);
private:
    unsigned int m_VBO, m_VAO;
    std::vector<Circle> m_programVertices;
};

#endif

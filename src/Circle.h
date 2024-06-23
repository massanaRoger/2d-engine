#ifndef CIRCLE_H
#define CIRCLE_H

#include <vector>

#include "glm/vec3.hpp"

struct Vertex {
    float x;
    float y;
    float z;

    Vertex(const float x, const float y, const float z);
};

class Circle {
public:
    explicit Circle(std::size_t n);

    std::vector<Vertex>& data();
    void insert(const Vertex& vertex);
    Vertex& operator[](std::size_t idx);
    const Vertex& operator[](std::size_t idx) const;
    std::size_t size();
    void updateTick(double descentSpeed);
    glm::vec3 &descentVector();

    using iterator = std::vector<Vertex>::iterator;
    using const_iterator = std::vector<Vertex>::const_iterator;
    iterator begin();
    [[nodiscard]] const_iterator begin() const;
    iterator end();
    [[nodiscard]] const_iterator end() const;

private:
    std::vector<Vertex> m_vertices;
    glm::vec3 m_descentVector;
};
#endif

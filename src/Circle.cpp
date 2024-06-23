#include "Circle.h"

Vertex::Vertex(const float x, const float y, const float z) : x{x}, y{y}, z{z} {}

Circle::Circle(std::size_t n) : m_descentVector(0.0f) {
    m_vertices = std::vector<Vertex>();
    m_vertices.reserve(n);
}

std::vector<Vertex>& Circle::data() {
    return m_vertices;
}

void Circle::insert(const Vertex& vertex) {
    m_vertices.push_back(vertex);
}

Vertex& Circle::operator[](std::size_t idx) {
    return m_vertices[idx];
}

const Vertex& Circle::operator[](std::size_t idx) const {
    return m_vertices[idx];
}

std::size_t Circle::size() {
    return m_vertices.size();
}

void Circle::updateTick(double descentSpeed) {
    m_descentVector.y -= descentSpeed;
}

glm::vec3 &Circle::descentVector() {
    return m_descentVector;
}

using iterator = std::vector<Vertex>::iterator;
using const_iterator = std::vector<Vertex>::const_iterator;

iterator Circle::begin() {
    return m_vertices.begin();
}

const_iterator Circle::begin() const {
    return m_vertices.begin();
}

iterator Circle::end() {
    return m_vertices.end();
}

const_iterator Circle::end() const {
    return m_vertices.end();
}

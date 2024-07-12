#include <string>
#include "utils.h"

#include "glm/geometric.hpp"

std::string getFullPath(const std::string& filename) {
    return std::string(CURRENT_WORKING_DIR) + "/" + filename;
}

std::vector<glm::vec3> calculateNormals(const std::vector<glm::vec3> &vertices) {
    std::vector<glm::vec3> normals;
    size_t numVertices = vertices.size();
    normals.reserve(numVertices);

    for (size_t i = 0; i < numVertices; i++) {
        glm::vec3 current = vertices[i];
        glm::vec3 next = vertices[(i + 1) % numVertices];

        glm::vec3 edge = next - current;

        glm::vec3 normal = glm::vec3(-edge.y, edge.x, 0.0f);

        normal = glm::normalize(normal);

        normals.push_back(normal);
    }

    return normals;
}

void projectPolygon(const std::vector<glm::vec3>& vertices, const glm::vec3& axis, float& min, float& max) {
    min = max = glm::dot(axis, vertices[0]);

    for (const auto& vertex : vertices) {
        float projection = glm::dot(axis, vertex);
        if (projection < min) {
            min = projection;
        }
        if (projection > max) {
            max = projection;
        }
    }
}

bool overlapOnAxis(const std::vector<glm::vec3>& vertices1, const std::vector<glm::vec3>& vertices2, const glm::vec3& axis) {
    float min1, max1;
    projectPolygon(vertices1, axis, min1, max1);
    float min2, max2;
    projectPolygon(vertices2, axis, min2, max2);

    return (max1 >= min2 && max2 >= min1);
}

std::vector<glm::vec3> calculateAABBvertices(const glm::vec2 &min, const glm::vec2 &max) {
    std::vector<glm::vec3> vertices;

    vertices.reserve(4);

    vertices.emplace_back(min, 0.0f);
    vertices.emplace_back(max.x, min.y, 0.0f);
    vertices.emplace_back(max, 0.0f);
    vertices.emplace_back(min.x, max.y, 0.0f);

    return vertices;
}

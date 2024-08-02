
#include "Polygon.h"

Polygon::Polygon(VelocityComponent *vc, AccelerationComponent *ac, MassComponent *mc, PolygonComponent *pc, AngularVelocityComponent *avc, AngularAccelerationComponent *aac, InertiaComponent *ic) : vc(vc), ac(ac), mc(mc), pc(pc), avc(avc), aac(aac), ic(ic) {}

float calculateArea(const std::vector<glm::vec3>& vertices) {
    float area = 0.0f;
    int N = vertices.size();
    for (int i = 0; i < N; ++i) {
        const glm::vec3& p1 = vertices[i];
        const glm::vec3& p2 = vertices[(i + 1) % N];
        area += p1.x * p2.y - p2.x * p1.y;
    }
    return std::abs(area) / 2.0f;
}

glm::vec3 calculateCentroid(const std::vector<glm::vec3>& vertices, float area) {
    glm::vec3 centroid(0.0f, 0.0f, 0.0f);
    int N = vertices.size();
    float factor = 1.0f / (6.0f * area);
    for (int i = 0; i < N; ++i) {
        const glm::vec3& p1 = vertices[i];
        const glm::vec3& p2 = vertices[(i + 1) % N];
        float common = p1.x * p2.y - p2.x * p1.y;
        centroid.x += (p1.x + p2.x) * common;
        centroid.y += (p1.y + p2.y) * common;
    }
    centroid *= factor;
    return centroid;
}


float Polygon::calculateRotationalInertia(const std::vector<glm::vec3> &vertices, float inverseMass) {
    int N = vertices.size();
    if (N < 3) return 0.0f;

    float area = calculateArea(vertices);
    glm::vec3 centroid = calculateCentroid(vertices, area);

    std::vector<glm::vec3> shiftedVertices(N);
    for (int i = 0; i < N; ++i) {
        shiftedVertices[i] = vertices[i] - centroid;
    }

    float mass = 1.0f / inverseMass;
    float density = mass / area;

    float inertia = 0.0f;
    for (int i = 0; i < N; ++i) {
        const glm::vec3& p1 = shiftedVertices[i];
        const glm::vec3& p2 = shiftedVertices[(i + 1) % N];

        float common_factor = p1.x * p2.y - p2.x * p1.y;
        inertia += (p1.y * p1.y + p1.y * p2.y + p2.y * p2.y) * (p1.x * p2.x) -
                   (p1.x * p1.x + p1.x * p2.x + p2.x * p2.x) * (p1.y * p2.y);
    }

    inertia *= density / 12.0f;
    return std::abs(inertia);

}



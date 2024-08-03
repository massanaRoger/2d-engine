#ifndef UTILS_H
#define UTILS_H
#include "Polygon.h"
#include "glm/fwd.hpp"
#include <string>
#include <vector>

// Get path from project source
std::string getFullPath(const std::string& filename);

std::vector<glm::vec3> calculateNormals(const std::vector<glm::vec3> &vertices);

void projectPolygon(const std::vector<glm::vec3>& vertices, const glm::vec3& axis, float& min, float& max);
void projectCircle(const glm::vec3 &center, float radius, glm::vec3 &axis, float &min, float &max);

bool overlapOnAxis(const std::vector<glm::vec3>& vertices1, const std::vector<glm::vec3>& vertices2, const glm::vec3& axis);

std::vector<glm::vec3> calculateAABBvertices(const glm::vec3 &min, const glm::vec3 &max);

#endif

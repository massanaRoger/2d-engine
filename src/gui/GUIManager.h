#pragma once

#include <glm/glm.hpp>
#include "imgui.h"

class GUIManager {
public:
    GUIManager();

    void Render();
    [[nodiscard]] glm::vec3 GetSelectedColor() const;

private:
    glm::vec3 m_SelectedColor;
};

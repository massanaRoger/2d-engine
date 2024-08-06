//
// Created by Roger2 on 06/08/2024.
//

#include "GUIManager.h"

GUIManager::GUIManager(): m_SelectedColor(1.0f, 1.0f, 1.0f) {}

void GUIManager::Render() {
    ImGui::Begin("Color Picker");
    ImGui::ColorEdit3("Selected Color", (float*)&m_SelectedColor);
    ImGui::End();
}

glm::vec3 GUIManager::GetSelectedColor() const {
    return m_SelectedColor;
}


#include "pch.h"
#include "ImMessageBox.h"

void ImMessageBox::Show(const std::u8string& message, const std::u8string& title, std::function<void(ImMessageBoxResult)> callback) {
    m_message = message;
    m_title = title;
    m_callback = callback;
    m_ShouldOpen = true;
}

void ImMessageBox::Show(const std::wstring& message, const std::wstring& title, std::function<void(ImMessageBoxResult)> callback) {
    Show(wstringToU8String(message), wstringToU8String(title), callback);
}

void ImMessageBox::Render() {
    if (m_ShouldOpen) {
        ImGui::OpenPopup(reinterpret_cast<const char*>(m_title.c_str()));
        m_ShouldOpen = false;
    }

    ImVec2 center = ImGui::GetMainViewport()->GetCenter();
    ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));

    if (ImGui::BeginPopupModal(reinterpret_cast<const char*>(m_title.c_str()), NULL, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::TextWrapped("%s", reinterpret_cast<const char*>(m_message.c_str()));
        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();

        if (ImGui::Button("OK", ImVec2(120, 0))) {
            if (m_callback) m_callback(ImMessageBoxResult::Ok);
            ImGui::CloseCurrentPopup();
        }

        ImGui::SameLine();

        if (ImGui::Button("Cancel", ImVec2(120, 0))) {
            if (m_callback) m_callback(ImMessageBoxResult::Cancel);
            ImGui::CloseCurrentPopup();
        }

        ImGui::EndPopup();
    }
}
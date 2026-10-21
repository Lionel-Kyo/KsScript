#include "pch.h"
#include "ChangeKeyAndModePopup.h"

ChangeKeyAndModePopUp::ChangeKeyAndModePopUp() {
    m_orderedKeys.assign(NAME_MATCH_KEYS.begin(), NAME_MATCH_KEYS.end());
    std::sort(m_orderedKeys.begin(), m_orderedKeys.end(), [](const auto& a, const auto& b) {
        return a.second < b.second;
    });

    for (int i = 0; i < m_orderedKeys.size(); i++) {
        if (m_orderedKeys[i].first == u8"F12") {
            m_defaultSelectedStartKeyIndex = i;
            m_defaultSelectedEndKeyIndex = i;
            break;
        }
    }
}

void ChangeKeyAndModePopUp::Open(
    const char* name,
    std::optional<ScriptRunMode> runMode,
    std::optional<HotkeyData> startHotkey,
    std::optional<HotkeyData> endHotkey
) {
    m_popupName = name;
    m_runMode = runMode;
    m_startHotkey = startHotkey;
    m_endHotkey = endHotkey;

    ImGui::OpenPopup(m_popupName);
}

void ChangeKeyAndModePopUp::ApplyHotkeyDefaults(const HotkeyData& hk, int& keyIdx, int& ctrlIdx, int& shiftIdx, int& altIdx) {
    for (int i = 0; i < m_orderedKeys.size(); i++) {
        if (m_orderedKeys[i].second == hk.key) {
            keyIdx = i;
            break;
        }
    }
    if (hk.ctrl == KeyModifierRequirement::MustUp) ctrlIdx = 1;
    else if (hk.ctrl == KeyModifierRequirement::MustDown) ctrlIdx = 2;
    else ctrlIdx = 0;

    if (hk.shift == KeyModifierRequirement::MustUp) shiftIdx = 1;
    else if (hk.shift == KeyModifierRequirement::MustDown) shiftIdx = 2;
    else shiftIdx = 0;

    if (hk.alt == KeyModifierRequirement::MustUp) altIdx = 1;
    else if (hk.alt == KeyModifierRequirement::MustDown) altIdx = 2;
    else altIdx = 0;
}

void ChangeKeyAndModePopUp::RenderPopup(
    ImGuiViewport* viewport,
    bool* pOpen,
    std::function<void(
        std::u8string startKey,
        std::vector<std::u8string> startKeyModifiers,
        std::u8string endKey,
        std::vector<std::u8string> endKeyModifiers,
        std::u8string runMode
        )> onSave
) {
    static const char* ctrlOptions[] = { "-", "CtrlUp", "CtrlDown" };
    static const char* shiftOptions[] = { "-", "ShiftUp", "ShiftDown" };
    static const char* altOptions[] = { "-", "AltUp", "AltDown" };

    ImGui::SetNextWindowPos(viewport->GetCenter(), ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
    if (ImGui::BeginPopupModal(m_popupName, pOpen, ImGuiWindowFlags_AlwaysAutoResize)) {
        if (ImGui::IsWindowAppearing()) {
            m_selectedStartKeyIndex = m_defaultSelectedStartKeyIndex;
            m_selectedStartCtrlIndex = 0;
            m_selectedStartShiftIndex = 0;
            m_selectedStartAltIndex = 0;

            m_selectedEndKeyIndex = m_defaultSelectedEndKeyIndex;
            m_selectedEndCtrlIndex = 0;
            m_selectedEndShiftIndex = 0;
            m_selectedEndAltIndex = 0;

            m_selectedRunModeIndex = 0;

            if (m_runMode.has_value()) {
                m_selectedRunModeIndex = static_cast<int>(m_runMode.value());
            }
            if (m_startHotkey.has_value()) {
                ApplyHotkeyDefaults(m_startHotkey.value(), m_selectedStartKeyIndex, m_selectedStartCtrlIndex, m_selectedStartShiftIndex, m_selectedStartAltIndex);
            }
            if (m_endHotkey.has_value()) {
                ApplyHotkeyDefaults(m_endHotkey.value(), m_selectedEndKeyIndex, m_selectedEndCtrlIndex, m_selectedEndShiftIndex, m_selectedEndAltIndex);
            }
        }

        ImGui::AlignTextToFramePadding();
#ifdef _LANG_ZH_TW_
        ImGui::Text("開始按鍵:");
#else
        ImGui::Text("StartKey:");
#endif
        ImGui::SameLine();
        ImGui::SetNextItemWidth(90.0f);
        if (ImGui::BeginCombo("##StartKey", reinterpret_cast<const char*>(m_orderedKeys[m_selectedStartKeyIndex].first.c_str()))) {
            for (int i = 0; i < m_orderedKeys.size(); i++) {
                const bool isSelected = m_selectedStartKeyIndex == i;
                if (ImGui::Selectable(reinterpret_cast<const char*>(m_orderedKeys[i].first.c_str()), isSelected)) {
                    m_selectedStartKeyIndex = i;
                }
                if (isSelected) {
                    ImGui::SetItemDefaultFocus();
                }
            }
            ImGui::EndCombo();
        }
        ImGui::SameLine();
        ImGui::SetNextItemWidth(110.0f);
        ImGui::Combo("##StartCtrl", &m_selectedStartCtrlIndex, ctrlOptions, IM_ARRAYSIZE(ctrlOptions));
        ImGui::SameLine();
        ImGui::SetNextItemWidth(110.0f);
        ImGui::Combo("##StartShift", &m_selectedStartShiftIndex, shiftOptions, IM_ARRAYSIZE(shiftOptions));
        ImGui::SameLine();
        ImGui::SetNextItemWidth(110.0f);
        ImGui::Combo("##StartAlt", &m_selectedStartAltIndex, altOptions, IM_ARRAYSIZE(altOptions));

        bool isSwitchMode = (static_cast<ScriptRunMode>(m_selectedRunModeIndex) == ScriptRunMode::FullSwitch ||
            static_cast<ScriptRunMode>(m_selectedRunModeIndex) == ScriptRunMode::Switch);

        ImGui::AlignTextToFramePadding();
        if (!isSwitchMode) ImGui::BeginDisabled();
#ifdef _LANG_ZH_TW_
        ImGui::Text("停止按鍵:");
#else
        ImGui::Text("EndKey:");
#endif
        ImGui::SameLine();
        ImGui::SetNextItemWidth(90.0f);
        if (ImGui::BeginCombo("##EndKey", reinterpret_cast<const char*>(m_orderedKeys[m_selectedEndKeyIndex].first.c_str()))) {
            for (int i = 0; i < m_orderedKeys.size(); i++) {
                const bool isSelected = m_selectedEndKeyIndex == i;
                if (ImGui::Selectable(reinterpret_cast<const char*>(m_orderedKeys[i].first.c_str()), isSelected)) {
                    m_selectedEndKeyIndex = i;
                }
                if (isSelected) {
                    ImGui::SetItemDefaultFocus();
                }
            }
            ImGui::EndCombo();
        }
        ImGui::SameLine();
        ImGui::SetNextItemWidth(110.0f);
        ImGui::Combo("##EndCtrl", &m_selectedEndCtrlIndex, ctrlOptions, IM_ARRAYSIZE(ctrlOptions));
        ImGui::SameLine();
        ImGui::SetNextItemWidth(110.0f);
        ImGui::Combo("##EndShift", &m_selectedEndShiftIndex, shiftOptions, IM_ARRAYSIZE(shiftOptions));
        ImGui::SameLine();
        ImGui::SetNextItemWidth(110.0f);
        ImGui::Combo("##EndAlt", &m_selectedEndAltIndex, altOptions, IM_ARRAYSIZE(altOptions));
        if (!isSwitchMode) ImGui::EndDisabled();

        ImGui::AlignTextToFramePadding();
#ifdef _LANG_ZH_TW_
        ImGui::Text("運行模式:");
#else
        ImGui::Text("RunMode:");
#endif
        ImGui::SameLine();
        ImGui::SetNextItemWidth(150.0f);
        if (ImGui::BeginCombo("##RunMode", reinterpret_cast<const char*>(ScriptRunModeToString(static_cast<ScriptRunMode>(m_selectedRunModeIndex)).c_str()))) {
            for (int i = 0; i < static_cast<int>(static_cast<int32_t>(ScriptRunMode::Switch) + 1); i++) {
                const bool isSelected = m_selectedRunModeIndex == i;
                if (ImGui::Selectable(reinterpret_cast<const char*>(ScriptRunModeToString(static_cast<ScriptRunMode>(i)).c_str()), isSelected)) {
                    m_selectedRunModeIndex = i;
                }
                if (isSelected) {
                    ImGui::SetItemDefaultFocus();
                }
            }
            ImGui::EndCombo();
        }

        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();

#ifdef _LANG_ZH_TW_
        if (ImGui::Button("保存", ImVec2(120, 0))) {
#else
        if (ImGui::Button("Save", ImVec2(120, 0))) {
#endif
            auto buildKeyModifiers = [&](int ctrlIdx, int shiftIdx, int altIdx) {
                std::vector<std::u8string> result;
                if (ctrlIdx > 0) result.push_back(reinterpret_cast<const char8_t*>(ctrlOptions[ctrlIdx]));
                if (shiftIdx > 0) result.push_back(reinterpret_cast<const char8_t*>(shiftOptions[shiftIdx]));
                if (altIdx > 0) result.push_back(reinterpret_cast<const char8_t*>(altOptions[altIdx]));
                return result;
            };

            if (!isSwitchMode) {
                m_selectedEndKeyIndex = 0;
                m_selectedEndCtrlIndex = 0;
                m_selectedEndShiftIndex = 0;
                m_selectedEndAltIndex = 0;
            }

            std::u8string startKey = m_orderedKeys[m_selectedStartKeyIndex].first;
            std::vector<std::u8string> startKeyModifiers = buildKeyModifiers(m_selectedStartCtrlIndex, m_selectedStartShiftIndex, m_selectedStartAltIndex);
            std::u8string endKey = m_orderedKeys[m_selectedEndKeyIndex].first;
            std::vector<std::u8string> endKeyModifiers = buildKeyModifiers(m_selectedEndCtrlIndex, m_selectedEndShiftIndex, m_selectedEndAltIndex);
            std::u8string runModeStr = ScriptRunModeToString(static_cast<ScriptRunMode>(m_selectedRunModeIndex));

            if (onSave != nullptr) onSave(startKey, startKeyModifiers, endKey, endKeyModifiers, runModeStr);
            ImGui::CloseCurrentPopup();
        }
        ImGui::SameLine();
#ifdef _LANG_ZH_TW_
        if (ImGui::Button("取消", ImVec2(120, 0))) {
#else
        if (ImGui::Button("Cancel", ImVec2(120, 0))) {
#endif
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }
}
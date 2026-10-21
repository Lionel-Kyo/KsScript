#pragma once
#ifndef _CHANGE_KEY_AND_MODE_POPUP_H_
#define _CHANGE_KEY_AND_MODE_POPUP_H_

#include <optional>
#include <functional>
#include <vector>
#include <string>
#include <algorithm>
#include "libs/ImGui/imgui.h"
#include "Utils.h"

class ChangeKeyAndModePopUp {
public:
    // Constructor handles sorting NAME_MATCH_KEYS and finding F12 defaults once
    ChangeKeyAndModePopUp();

    // Call this once when you want to open the popup with new parameters
    void Open(
        const char* name,
        std::optional<ScriptRunMode> runMode,
        std::optional<HotkeyData> startHotkey,
        std::optional<HotkeyData> endHotkey
    );

    // Call this every frame in your main UI loop to render the modal
    void RenderPopup(
        ImGuiViewport* viewport,
        bool* pOpen,
        std::function<void(
            std::u8string startKey,
            std::vector<std::u8string> startKeyModifiers,
            std::u8string endKey,
            std::vector<std::u8string> endKeyModifiers,
            std::u8string runMode
            )> onSave
    );

private:
    void ApplyHotkeyDefaults(const HotkeyData& hk, int& keyIdx, int& ctrlIdx, int& shiftIdx, int& altIdx);

    std::vector<std::pair<std::u8string, int32_t>> m_orderedKeys;
    int m_defaultSelectedStartKeyIndex = 0;
    int m_defaultSelectedEndKeyIndex = 0;

    const char* m_popupName = "Change Key and Mode";
    std::optional<ScriptRunMode> m_runMode;
    std::optional<HotkeyData> m_startHotkey;
    std::optional<HotkeyData> m_endHotkey;

    int m_selectedStartKeyIndex = 0;
    int m_selectedStartCtrlIndex = 0;
    int m_selectedStartShiftIndex = 0;
    int m_selectedStartAltIndex = 0;

    int m_selectedEndKeyIndex = 0;
    int m_selectedEndCtrlIndex = 0;
    int m_selectedEndShiftIndex = 0;
    int m_selectedEndAltIndex = 0;

    int m_selectedRunModeIndex = 0;
};

#endif // !_CHANGE_KEY_AND_MODE_POPUP_H_

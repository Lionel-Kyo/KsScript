#include "pch.h"
#include "KeysRefWnd.h"

void KeysRefWnd::RenderWindow(HWND hwnd, bool* pOpen) {
    ImGuiViewport* viewport = ImGui::GetMainViewport();

    ImVec2 windowSize(450, 400);
    ImVec2 windowPos(viewport->GetCenter().x - windowSize.x * 0.5f, viewport->GetCenter().y - windowSize.y * 0.5f);

    ImGui::SetNextWindowSize(windowSize, ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowPos(windowPos, ImGuiCond_FirstUseEver);

#ifdef _LANG_ZH_TW_
    if (!ImGui::Begin("鍵盤按鍵說明", pOpen)) {
        ImGui::End();
        return;
    }
#else
    if (!ImGui::Begin("Keys Reference", pOpen)) {
        ImGui::End();
        return;
    }
#endif

    if (ImGui::IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows) &&
        ImGui::IsKeyPressed(ImGuiKey_Escape)) {
        *pOpen = false;
    }

    static std::vector<std::pair<std::u8string, int32_t>> s_orderedKeys;
    static bool s_initialized = false;
    if (!s_initialized) {
        s_orderedKeys.assign(NAME_MATCH_KEYS.begin(), NAME_MATCH_KEYS.end());
        std::sort(s_orderedKeys.begin(), s_orderedKeys.end(), [](const auto& a, const auto& b) {
            return a.second < b.second;
            });
        s_initialized = true;
    }

    static std::vector<bool> s_selectedRows;
    static int s_lastSelectedIdx = -1; // Shift-click range selection
    if (s_selectedRows.size() != s_orderedKeys.size()) {
        s_selectedRows.resize(s_orderedKeys.size(), false);
    }

    // Ctrl
    if (ImGui::IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows) && ImGui::GetIO().KeyCtrl && ImGui::IsKeyPressed(ImGuiKey_A)) {
        std::fill(s_selectedRows.begin(), s_selectedRows.end(), true);
    }

    ImGuiTableFlags flags = ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_ScrollY | ImGuiTableFlags_Resizable;

#ifdef _LANG_ZH_TW_
    if (ImGui::BeginTable("KeysRefTable", 2, flags, ImGui::GetContentRegionAvail())) {
        ImGui::TableSetupColumn("名稱", ImGuiTableColumnFlags_WidthStretch);
        ImGui::TableSetupColumn("數值", ImGuiTableColumnFlags_WidthFixed, 100.0f);
        ImGui::TableHeadersRow();
#else
    if (ImGui::BeginTable("KeysRefTable", 2, flags, ImGui::GetContentRegionAvail())) {
        ImGui::TableSetupColumn("Name", ImGuiTableColumnFlags_WidthStretch);
        ImGui::TableSetupColumn("Value", ImGuiTableColumnFlags_WidthFixed, 100.0f);
        ImGui::TableHeadersRow();
#endif

        for (size_t i = 0; i < s_orderedKeys.size(); ++i) {
            const auto& [key, value] = s_orderedKeys[i];
            ImGui::PushID(static_cast<int>(i));

            ImGui::TableNextRow();

            ImGui::TableSetColumnIndex(0);
            char label[64];
            sprintf_s(label, "##row_%zu", i);

            bool isSelected = s_selectedRows[i];
            if (ImGui::Selectable(label, isSelected, ImGuiSelectableFlags_SpanAllColumns | ImGuiSelectableFlags_AllowOverlap)) {
                ImGuiIO& io = ImGui::GetIO();
                int currentIndex = static_cast<int>(i);

                if (io.KeyCtrl) {
                    s_selectedRows[i] = !s_selectedRows[i];
                    if (s_selectedRows[i]) {
                        s_lastSelectedIdx = currentIndex;
                    }
                }
                else if (io.KeyShift && s_lastSelectedIdx != -1) {
                    int start = (std::min)(s_lastSelectedIdx, currentIndex);
                    int end = (std::max)(s_lastSelectedIdx, currentIndex);
                    std::fill(s_selectedRows.begin(), s_selectedRows.end(), false);
                    for (int r = start; r <= end; ++r) {
                        s_selectedRows[r] = true;
                    }
                } else {
                    std::fill(s_selectedRows.begin(), s_selectedRows.end(), false);
                    s_selectedRows[i] = true;
                    s_lastSelectedIdx = currentIndex;
                }
            }

            ImGui::SameLine();
            ImGui::SetCursorPosX(ImGui::GetCursorPosX() - ImGui::GetStyle().ItemSpacing.x);
            ImGui::Text("%s", reinterpret_cast<const char*>(key.c_str()));

            ImGui::TableSetColumnIndex(1);
            std::string hexVal = std::format("0x{:X}", value);
            ImGui::Text("%s", hexVal.c_str());

            ImGui::PopID();
        }

        // Ctrl+C
        if (ImGui::IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows) && ImGui::GetIO().KeyCtrl && ImGui::IsKeyPressed(ImGuiKey_C)) {
            std::u8string clipboardText;
            for (size_t i = 0; i < s_orderedKeys.size(); ++i) {
                if (i < s_selectedRows.size() && s_selectedRows[i]) {
                    const auto& [key, value] = s_orderedKeys[i];
                    std::u8string hexVal = ToU8StringUnchecked(std::format("0x{:X}", value));
                    std::u8string rowStr = key + u8"\t" + hexVal;

                    if (!clipboardText.empty()) {
                        clipboardText += u8"\r\n";
                    }
                    clipboardText += rowStr;
                }
            }
            if (!clipboardText.empty()) {
                ImGui::SetClipboardText(reinterpret_cast<const char*>(clipboardText.c_str()));
            }
        }

        ImGui::EndTable();
    }

    ImGui::End();
}
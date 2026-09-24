#include "pch.h"
#include "FunctionsRefWnd.h"

#ifdef _LANG_ZH_TW_
static const std::vector<FuncInfo> s_funcInfos = {
    { u8"設定開始按鍵", u8"(按鍵, [modifiers...])", u8"設定腳本啟動觸發鍵。接受按鍵名稱或按鍵編號，可選的modifiers：CtrlDown、CtrlUp、ShiftDown、ShiftUp、AltDown、AltUp。" },
    { u8"設定停止按鍵", u8"(按鍵, [modifiers...])", u8"設定腳本停止觸發鍵。接受按鍵名稱或按鍵編號，可選的modifiers：CtrlDown、CtrlUp、ShiftDown、ShiftUp、AltDown、AltUp。" },
    { u8"設定運行模式", u8"(模式)", u8"設定運行模式。有效模式：完整單次、完整連續、完整切換、單次、連續、切換。" },
    { u8"延遲", u8"(延遲毫秒)", u8"將腳本執行暫停指定的毫秒。" },
    { u8"滑鼠按下", u8"(滑鼠按鍵)", u8"按下指定的滑鼠按鍵。有效滑鼠按鍵：左、右、中、X1、X2。" },
    { u8"滑鼠放開", u8"(滑鼠按鍵)", u8"放開指定的滑鼠按鍵。有效滑鼠按鍵：左、右、中、X1、X2。" },
    { u8"絕對移動", u8"(x, y)", u8"將滑鼠游標移動至指定的絕對螢幕座標 (x, y)。" },
    { u8"相對移動", u8"(dx, dy)", u8"相對於目前位置，依 (dx, dy) 移動滑鼠游標。" },
    { u8"鍵盤按下", u8"(按鍵)", u8"按下鍵盤按鍵，接受按鍵名稱或按鍵編號。" },
    { u8"鍵盤放開", u8"(按鍵)", u8"放開鍵盤按鍵，接受按鍵名稱或按鍵編號。" },
    { u8"滾輪下", u8"()", u8"向下滾動滑鼠滾輪。此指令不需要任何引數。" },
    { u8"滾輪上", u8"()", u8"向上滾動滑鼠滾輪。此指令不需要任何引數。" },
    { u8"按住視窗標題", u8"(視窗名稱)", u8"模擬按住指定視窗的標題列。" },
    { u8"放開視窗標題", u8"(視窗名稱)", u8"模擬放開指定視窗的標題列。" },
    { u8"輸出文字", u8"(文字, [延遲毫秒])", u8"輸出文字，可選填字元間的延遲毫秒數。單引號 ('...') 表示字面值；雙引號 (\"...\" 表示escape sequences。" },
    { u8"顯示座標", u8"()", u8"顯示目前的螢幕滑鼠座標。此指令不需要任何引數。" }
};
#else
static const std::vector<FuncInfo> s_funcInfos = {
    { u8"SetStartKey", u8"(key, [modifiers...])", u8"Defines the script start trigger. Accepts a key name or VK code, with optional modifier states (CtrlDown, CtrlUp, ShiftDown, ShiftUp, AltDown, AltUp)." },
    { u8"SetEndKey", u8"(key, [modifiers...])", u8"Defines the script stop trigger. Accepts a key name or VK code, with optional modifier states (CtrlDown, CtrlUp, ShiftDown, ShiftUp, AltDown, AltUp)." },
    { u8"SetRunMode", u8"(mode)", u8"Sets execution mode. Valid modes: FullSingle, FullContinuous, FullSwitch, Single, Continuous, or Switch." },
    { u8"Delay", u8"(ms)", u8"Pauses script execution for the specified duration in milliseconds." },
    { u8"MouseDown", u8"(buttonName)", u8"Presses a mouse button down. Valid buttons: Left, Right, Middle, X1, X2." },
    { u8"MouseUp", u8"(buttonName)", u8"Releases a mouse button. Valid buttons: Left, Right, Middle, X1, X2." },
    { u8"AbsoluteMove", u8"(x, y)", u8"Moves the mouse cursor to absolute screen coordinates (x, y)." },
    { u8"RelativeMove", u8"(dx, dy)", u8"Moves the mouse cursor relative to its current position by (dx, dy)." },
    { u8"KeyboardDown", u8"(key)", u8"Presses down a keyboard key using a key name or virtual-key (VK) code." },
    { u8"KeyboardUp", u8"(key)", u8"Releases a keyboard key using a key name or virtual-key (VK) code." },
    { u8"WheelDown", u8"()", u8"Scrolls the mouse wheel down. This command takes no arguments." },
    { u8"WheelUp", u8"()", u8"Scrolls the mouse wheel up. This command takes no arguments." },
    { u8"HoldTitleBar", u8"(windowName)", u8"Simulates holding down the title bar of the specified window." },
    { u8"ReleaseTitleBar", u8"(windowName)", u8"Simulates release the title bar of the specified window. }" },
    { u8"OutputText", u8"(text, [delayMs])", u8"Outputs a text string with an optional delay in milliseconds between characters. Single quotes ('...') are literal; double quotes (\"...\" support escape sequences." },
    { u8"ShowCoordinate", u8"()", u8"Displays the current cursor screen coordinates. This command takes no arguments." }
};
#endif // _LANG_ZH_TW_

void FunctionsRefWnd::RenderWindow(HWND hwnd, bool* pOpen) {
    ImGuiViewport* viewport = ImGui::GetMainViewport();

    ImVec2 windowSize(550, 400);
    ImVec2 windowPos(viewport->GetCenter().x - windowSize.x * 0.5f, viewport->GetCenter().y - windowSize.y * 0.5f);

    ImGui::SetNextWindowSize(windowSize, ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowPos(windowPos, ImGuiCond_FirstUseEver);

#ifdef _LANG_ZH_TW_
    if (!ImGui::Begin("腳本指令說明", pOpen)) {
        ImGui::End();
        return;
    }
#else
    if (!ImGui::Begin("Functions Reference", pOpen)) {
        ImGui::End();
        return;
    }
#endif

    if (ImGui::IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows) &&
        ImGui::IsKeyPressed(ImGuiKey_Escape)) {
        *pOpen = false;
    }

    static std::vector<bool> s_selectedRows;
    static int s_lastSelectedIdx = -1; // Shift-click range selection
    if (s_selectedRows.size() != s_funcInfos.size()) {
        s_selectedRows.resize(s_funcInfos.size(), false);
    }

    // Ctrl
    if (ImGui::IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows) && ImGui::GetIO().KeyCtrl && ImGui::IsKeyPressed(ImGuiKey_A)) {
        std::fill(s_selectedRows.begin(), s_selectedRows.end(), true);
    }

    ImGuiTableFlags flags = ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_ScrollY | ImGuiTableFlags_Resizable;

#ifdef _LANG_ZH_TW_
    if (ImGui::BeginTable("FuncsRefTable", 3, flags, ImGui::GetContentRegionAvail())) {
        ImGui::TableSetupColumn("名稱", ImGuiTableColumnFlags_WidthFixed, 150.0f);
        ImGui::TableSetupColumn("引數", ImGuiTableColumnFlags_WidthFixed, 150.0f);
        ImGui::TableSetupColumn("說明", ImGuiTableColumnFlags_WidthStretch);
        ImGui::TableHeadersRow();
#else
    if (ImGui::BeginTable("FuncsRefTable", 3, flags, ImGui::GetContentRegionAvail())) {
        ImGui::TableSetupColumn("Name", ImGuiTableColumnFlags_WidthFixed, 150.0f);
        ImGui::TableSetupColumn("Parameters", ImGuiTableColumnFlags_WidthFixed, 150.0f);
        ImGui::TableSetupColumn("Explanation", ImGuiTableColumnFlags_WidthStretch);
        ImGui::TableHeadersRow();
#endif

        for (size_t i = 0; i < s_funcInfos.size(); ++i) {
            const auto& [name, parameters, explaination] = s_funcInfos[i];

            float explanationWidth = ImGui::GetContentRegionAvail().x;
            ImVec2 explanationSize = ImGui::CalcTextSize(
                reinterpret_cast<const char*>(explaination.c_str()),
                nullptr,
                false,
                explanationWidth
            );
            float rowHeight = (std::max)(ImGui::GetTextLineHeight(), explanationSize.y);
            ImGui::TableNextRow(ImGuiTableRowFlags_None, rowHeight);

            ImGui::TableSetColumnIndex(0);

            char label[64];
            sprintf_s(label, "##row_%zu", i);

            bool isSelected = s_selectedRows[i];

            if (ImGui::Selectable(label, isSelected, ImGuiSelectableFlags_SpanAllColumns | ImGuiSelectableFlags_AllowOverlap, ImVec2(0.0f, rowHeight))) {

                ImGuiIO& io = ImGui::GetIO();
                int currentIndex = static_cast<int>(i);

                if (io.KeyCtrl) {
                    s_selectedRows[i] = !s_selectedRows[i];
                    if (s_selectedRows[i])
                        s_lastSelectedIdx = currentIndex;
                }
                else if (io.KeyShift && s_lastSelectedIdx != -1) {
                    int start = (std::min)(s_lastSelectedIdx, currentIndex);
                    int end = (std::max)(s_lastSelectedIdx, currentIndex);

                    std::fill(s_selectedRows.begin(), s_selectedRows.end(), false);

                    for (int r = start; r <= end; ++r)
                        s_selectedRows[r] = true;
                } else {
                    std::fill(s_selectedRows.begin(), s_selectedRows.end(), false);
                    s_selectedRows[i] = true;
                    s_lastSelectedIdx = currentIndex;
                }
            }

            ImGui::SameLine();
            ImGui::SetCursorPosX(ImGui::GetCursorPosX() - ImGui::GetStyle().ItemSpacing.x);
            ImGui::Text("%s", name.c_str());

            ImGui::TableSetColumnIndex(1);
            ImGui::Text("%s", parameters.c_str());

            ImGui::TableSetColumnIndex(2);
            ImGui::TextWrapped("%s", explaination.c_str());
        }

        // Ctrl+C
        if (ImGui::IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows) && ImGui::GetIO().KeyCtrl && ImGui::IsKeyPressed(ImGuiKey_C)) {
            std::u8string clipboardText;
            for (size_t i = 0; i < s_funcInfos.size(); ++i) {
                if (i < s_funcInfos.size() && s_selectedRows[i]) {
                    const auto& [name, parameters, explaination] = s_funcInfos[i];
                    std::u8string rowStr = name + u8"\t" + parameters + u8"\t" + explaination;

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
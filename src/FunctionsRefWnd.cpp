#include "pch.h"
#include "FunctionsRefWnd.h"

void FunctionsRefWnd::RenderWindow(HWND hwnd, bool* pOpen) {
    ImGuiViewport* viewport = ImGui::GetMainViewport();

    ImVec2 windowSize(550, 400);
    ImVec2 windowPos(viewport->GetCenter().x - windowSize.x * 0.5f, viewport->GetCenter().y - windowSize.y * 0.5f);

    ImGui::SetNextWindowSize(windowSize, ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowPos(windowPos, ImGuiCond_FirstUseEver);

    if (!ImGui::Begin(I18n::Get(StringId::FunctionsRef), pOpen)) {
        ImGui::End();
        return;
    }

    if (ImGui::IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows) &&
        ImGui::IsKeyPressed(ImGuiKey_Escape)) {
        *pOpen = false;
    }

    const auto& funcInfos = I18n::GetFuncInfos();

    static std::vector<bool> s_selectedRows;
    static int s_lastSelectedIdx = -1; // Shift-click range selection
    if (s_selectedRows.size() != funcInfos.size()) {
        s_selectedRows.resize(funcInfos.size(), false);
    }

    // Ctrl+A
    if (ImGui::IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows) && ImGui::GetIO().KeyCtrl && ImGui::IsKeyPressed(ImGuiKey_A)) {
        std::fill(s_selectedRows.begin(), s_selectedRows.end(), true);
    }

    ImGuiTableFlags flags = ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_ScrollY | ImGuiTableFlags_Resizable;

    if (ImGui::BeginTable("FuncsRefTable", 3, flags, ImGui::GetContentRegionAvail())) {
        ImGui::TableSetupColumn(I18n::Get(StringId::ColName), ImGuiTableColumnFlags_WidthFixed, 150.0f);
        ImGui::TableSetupColumn(I18n::Get(StringId::ColParameters), ImGuiTableColumnFlags_WidthFixed, 150.0f);
        ImGui::TableSetupColumn(I18n::Get(StringId::ColExplanation), ImGuiTableColumnFlags_WidthStretch);
        ImGui::TableHeadersRow();

        for (size_t i = 0; i < funcInfos.size(); ++i) {
            const auto& [name, parameters, explaination] = funcInfos[i];

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
            for (size_t i = 0; i < funcInfos.size(); ++i) {
                if (i < funcInfos.size() && s_selectedRows[i]) {
                    const auto& [name, parameters, explaination] = funcInfos[i];
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
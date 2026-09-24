#include "pch.h"
#include "MainWnd.h"

bool MainWnd::OpenScriptFilesDialog(HWND hwnd, std::vector<std::u8string>& outPaths) {
    constexpr size_t buffSize = 32768;
    std::vector<wchar_t> buffer(buffSize, L'\0');

    OPENFILENAMEW ofn = { sizeof(ofn) };
    ofn.hwndOwner = hwnd;
    ofn.lpstrFilter = L"UTF-8 Text Scripts (*.txt)\0*.txt\0All Files (*.*)\0*.*\0";
    ofn.lpstrFile = buffer.data();
    ofn.nMaxFile = (DWORD)buffSize;

    ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_ALLOWMULTISELECT | OFN_EXPLORER | OFN_LONGNAMES;

    if (!GetOpenFileNameW(&ofn)) {
        return false;
    }

    const wchar_t* p = ofn.lpstrFile;
    std::wstring firstToken = p;
    p += firstToken.length() + 1;

    if (*p == L'\0') {
        // Single File
        outPaths.push_back(wstringToU8String(firstToken));
    }
    else {
        // Multiple Files
        if (!firstToken.empty() && firstToken.back() != L'\\') {
            firstToken += L'\\';
        }

        while (*p != L'\0') {
            std::wstring_view relativeName = p;

            std::wstring absolutePath = firstToken + std::wstring(relativeName);
            outPaths.push_back(wstringToU8String(absolutePath));

            p += relativeName.length() + 1;
        }
    }
    return true;
}


bool MainWnd::LoadScriptEntry(const std::u8string& filePath, std::u8string& outErrorContent, std::u8string& outErrorTitle, ScriptEntry& outEntry) {
    ScriptEntry entry;
    entry.filePath = filePath;
    entry.fileName = std::filesystem::path(filePath).filename().u8string();

    std::vector<std::u8string> fileLines;
    if (!ReadUtf8FileLines(entry.filePath, fileLines)) {
#ifdef _LANG_ZH_TW_
        outErrorContent = wstringToU8String(std::format(L"讀取UTF-8 .txt腳本文件失敗\r\n{}", u8stringToWString(entry.filePath)));
        outErrorTitle = u8"文件錯誤";
#else
        outErrorContent = wstringToU8String(std::format(L"Failed to read UTF-8 script file.\r\n{}", u8stringToWString(entry.filePath)));
        outErrorTitle = u8"File Error";
#endif // _LANG_ZH_TW_
        return false;
    }
    entry.lines = fileLines;

    std::vector<ParsedCommand> parsedCommands;
    HotkeyData startKey;
    HotkeyData endKey;
    ScriptRunMode runMode;
    std::u8string errorMsg;
    int errorLine = 0;
    if (!ValidateAndParseScript(fileLines, startKey, endKey, runMode, parsedCommands, errorMsg, errorLine)) {
        outErrorContent = errorMsg + u8"\r\n" + entry.filePath;
#ifdef _LANG_ZH_TW_
        outErrorTitle = u8"腳本錯誤";
#else
        outErrorTitle = u8"Script Syntax Error";
#endif // _LANG_ZH_TW_
        return false;
    }
    switch (runMode) {
    case ScriptRunMode::Single:
    case ScriptRunMode::FullSingle:
        endKey = HotkeyData();
        break;
    case ScriptRunMode::Continuous:
    case ScriptRunMode::FullContinuous:
        endKey = startKey;
        break;
    }
    entry.data.startKey = startKey;
    entry.data.endKey = endKey;
    entry.data.mode = runMode;
    entry.data.commands = parsedCommands;

    outEntry = std::move(entry);
    return true;
}

void MainWnd::RenderWindow(HWND hwnd) {
    //ImGui::SetNextWindowSize(ImVec2(640, 480), ImGuiCond_FirstUseEver);

    RECT rect;
    GetClientRect(hwnd, &rect);
    float width = static_cast<float>(rect.right - rect.left);
    float height = static_cast<float>(rect.bottom - rect.top);

    ImGui::SetNextWindowPos(ImVec2(0.0f, 0.0f));
    ImGui::SetNextWindowSize(ImVec2(width, height));

    ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoTitleBar |
        ImGuiWindowFlags_NoCollapse |
        ImGuiWindowFlags_NoResize |
        ImGuiWindowFlags_NoMove |
        ImGuiWindowFlags_NoBringToFrontOnFocus |
        ImGuiWindowFlags_NoNavFocus;

    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(16.0f, 16.0f));


#ifdef _LANG_ZH_TW_
    ImGui::Begin("腳本管理器", nullptr, window_flags);
#else
    ImGui::Begin("Script Manager", nullptr, window_flags);
#endif


#ifdef _LANG_ZH_TW_
    if (ImGui::Button("讀取腳本")) {
#else
    if (ImGui::Button("Open Scripts")) {
#endif
        std::vector<std::u8string> filePaths;
        if (OpenScriptFilesDialog(hwnd, filePaths)) {
            for (const auto& path : filePaths) {
                ScriptEntry entry;
                std::u8string errContent, errTitle;

                if (!LoadScriptEntry(path, errContent, errTitle, entry)) {
                    MsgBox().Show(errContent, errTitle);
                    break;
                }

                s_tableRows.push_back({
                    .isEnabled=false,
                    .filePath=entry.filePath,
                    .fileName=entry.fileName,
                    .mode=ScriptRunModeToString(entry.data.mode),
                    .startKey= HotkeyDataToString(entry.data.startKey),
                    .endKey= HotkeyDataToString(entry.data.endKey),
                });

                std::lock_guard<std::mutex> lock(MainHelper::GetInstance().LoadedScriptsMutex());
                auto& scripts = MainHelper::GetInstance().LoadedScripts();
                for (size_t i = 0; i < scripts.size(); ++i) {
                    try {
                        if (std::filesystem::equivalent(scripts[i].filePath, entry.filePath)) {
                            s_tableRows.erase(s_tableRows.begin() + i);
                            scripts.erase(scripts.begin() + i);
                        }
                    }
                    catch (const std::filesystem::filesystem_error& e) { }
                }
                scripts.push_back(std::move(entry));
            }
        }
    }

    ImGui::SameLine();
#ifdef _LANG_ZH_TW_
    if (ImGui::Button("腳本記錄器")) s_showRecordWnd = true;
    ImGui::SameLine();
    if (ImGui::Button("腳本指令說明")) s_showFunctionsRefWnd = true;
    ImGui::SameLine();
    if (ImGui::Button("鍵盤按鍵說明")) s_showKeysRefWnd = true;
#else
    if (ImGui::Button("Script Recorder")) s_showRecordWnd = true;
    ImGui::SameLine();
    if (ImGui::Button("Functions Reference")) s_showFunctionsRefWnd = true;
    ImGui::SameLine();
    if (ImGui::Button("Keys Reference")) s_showKeysRefWnd = true;
#endif

    ImGui::Separator();


    ImGuiTableFlags tableFlags = ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg |
        ImGuiTableFlags_Resizable | ImGuiTableFlags_ScrollY; // |
        //ImGuiTableFlags_HighlightHoveredColumn;

    if (ImGui::BeginTable("ScriptTable", 5, tableFlags, ImVec2(0, -ImGui::GetFrameHeightWithSpacing()))) {
#ifdef _LANG_ZH_TW_
        ImGui::TableSetupColumn("開啟", ImGuiTableColumnFlags_WidthFixed, 60.0f);
        ImGui::TableSetupColumn("腳本名稱", ImGuiTableColumnFlags_WidthStretch, 2.0f);
        ImGui::TableSetupColumn("運行模式", ImGuiTableColumnFlags_WidthFixed, 120.0f);
        ImGui::TableSetupColumn("開始按鍵", ImGuiTableColumnFlags_WidthFixed, 120.0f);
        ImGui::TableSetupColumn("停止按鍵", ImGuiTableColumnFlags_WidthFixed, 120.0f);
#else
        ImGui::TableSetupColumn("Enable", ImGuiTableColumnFlags_WidthFixed, 60.0f);
        ImGui::TableSetupColumn("Script Name", ImGuiTableColumnFlags_WidthStretch, 2.0f);
        ImGui::TableSetupColumn("Execution Mode", ImGuiTableColumnFlags_WidthFixed, 120.0f);
        ImGui::TableSetupColumn("Start Key", ImGuiTableColumnFlags_WidthFixed, 120.0f);
        ImGui::TableSetupColumn("End Key", ImGuiTableColumnFlags_WidthFixed, 120.0f);
#endif
        ImGui::TableHeadersRow();

        int indexToRemove = -1;
        int indexToReload = -1;
        int indexToOpenScriptLocation = -1;
        static int selectedRow = -1;

        for (size_t i = 0; i < s_tableRows.size(); ++i) {
            auto& row = s_tableRows[i];
            ImGui::PushID(static_cast<int>(i));

            ImGui::TableNextRow();

            ImGui::TableSetColumnIndex(0);
            ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(2.0f, 2.0f));

            float checkBoxSize = ImGui::GetFrameHeight();
            float cellMinX = ImGui::GetCursorScreenPos().x;
            float cellMaxX = cellMinX + ImGui::GetContentRegionAvail().x;
            float centerX = (cellMinX + cellMaxX) * 0.5f;
            ImGui::SetCursorScreenPos(ImVec2(centerX - checkBoxSize * 0.5f, ImGui::GetCursorScreenPos().y));

            bool isEnabled = row.isEnabled;

            if (ImGui::Checkbox("##en", &isEnabled)) {
                row.isEnabled = isEnabled;
                std::lock_guard<std::mutex> lock(MainHelper::GetInstance().LoadedScriptsMutex());
                auto& entry = MainHelper::GetInstance().LoadedScripts().at(i);
                entry.data.isEnabled = isEnabled;
                if (!isEnabled) {
                    StopScriptExecution(entry.data, true, false);
                }
            }
            ImGui::PopStyleVar();

            ImGui::TableSetColumnIndex(1);
            char label[32];
            sprintf_s(label, "##row_%d", static_cast<int>(i));

            bool isSelected = (selectedRow == static_cast<int>(i));
            ImGui::PushStyleColor(ImGuiCol_Header, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
            ImGui::PushStyleColor(ImGuiCol_HeaderHovered, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
            ImGui::PushStyleColor(ImGuiCol_HeaderActive, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
            if (ImGui::Selectable(label, isSelected, ImGuiSelectableFlags_SpanAllColumns | ImGuiSelectableFlags_AllowOverlap)) {
                selectedRow = static_cast<int>(i);
            }
            ImGui::PopStyleColor(3);

            if (ImGui::BeginPopupContextItem("ScriptRowContext")) {
                selectedRow = static_cast<int>(i); // Select row on right-click
                if (row.isEnabled) {
#ifdef _LANG_ZH_TW_
                    if (ImGui::MenuItem("關閉腳本")) {
#else
                    if (ImGui::MenuItem("Disable")) {
#endif
                        std::lock_guard<std::mutex> lock(MainHelper::GetInstance().LoadedScriptsMutex());
                        auto& entry = MainHelper::GetInstance().LoadedScripts().at(i);
                        entry.data.isEnabled = false;
                        row.isEnabled = false;
                        StopScriptExecution(entry.data, true, false); 
                    }
                } else {
#ifdef _LANG_ZH_TW_
                    if (ImGui::MenuItem("開啟腳本")) {
#else
                    if (ImGui::MenuItem("Enable")) {
#endif
                        std::lock_guard<std::mutex> lock(MainHelper::GetInstance().LoadedScriptsMutex());
                        auto& entry = MainHelper::GetInstance().LoadedScripts().at(i);
                        entry.data.isEnabled = true;
                        row.isEnabled = true;
                    }
                }
                ImGui::Separator();
                if (s_testScript.isRunning) {
#ifdef _LANG_ZH_TW_
                    if (ImGui::MenuItem("停止測試執行")) {
#else
                    if (ImGui::MenuItem("Stop Test Run")) {
#endif
                        StopScriptExecution(s_testScript, true, false);
                    }
                }
                else {
#ifdef _LANG_ZH_TW_
                    if (ImGui::MenuItem("測試執行")) {
#else
                    if (ImGui::MenuItem("Run Test")) {
#endif
                        std::lock_guard<std::mutex> lock(MainHelper::GetInstance().LoadedScriptsMutex());
                        auto& entry = MainHelper::GetInstance().LoadedScripts().at(i);
                        s_testScript.commands = entry.data.commands;
                        s_testScript.mode = ScriptRunMode::FullSingle;
                        s_testScript.isEnabled = true;
                        StartScriptExecution(s_testScript);
                    }
                }
                ImGui::Separator();
#ifdef _LANG_ZH_TW_
                if (ImGui::MenuItem("移除腳本")) {
#else
                if (ImGui::MenuItem("Remove")) {
#endif
                    indexToRemove = static_cast<int>(i); 
                }
#ifdef _LANG_ZH_TW_
                if (ImGui::MenuItem("重新讀取")) {
#else
                if (ImGui::MenuItem("Reload")) {
#endif
                    indexToReload = static_cast<int>(i); 
                }
                ImGui::Separator();
#ifdef _LANG_ZH_TW_
                if (ImGui::MenuItem("打開腳本位置")) {
#else
                if (ImGui::MenuItem("Open Script Location")) {
#endif
                    indexToOpenScriptLocation = static_cast<int>(i);
                }
                ImGui::EndPopup();
            }

            ImGui::SameLine();
            ImGui::SetCursorPosX(ImGui::GetCursorPosX() - ImGui::GetStyle().ItemSpacing.x);
            ImGui::Text("%s", reinterpret_cast<const char*>(row.fileName.c_str()));

            ImGui::TableSetColumnIndex(2);
            ImGui::Text("%s", reinterpret_cast<const char*>(row.mode.c_str()));

            ImGui::TableSetColumnIndex(3);
            ImGui::Text("%s", reinterpret_cast<const char*>(row.startKey.c_str()));

            ImGui::TableSetColumnIndex(4);
            ImGui::Text("%s", reinterpret_cast<const char*>(row.endKey.c_str()));

            ImGui::PopID();
        }

        if (indexToRemove != -1) {
            std::lock_guard<std::mutex> lock(MainHelper::GetInstance().LoadedScriptsMutex());
            auto& scripts = MainHelper::GetInstance().LoadedScripts();
            StopScriptExecution(scripts[indexToRemove].data, true, false);
            scripts.erase(scripts.begin() + indexToRemove);
            s_tableRows.erase(s_tableRows.begin() + indexToRemove);
        }
        if (indexToReload != -1) {
            std::lock_guard<std::mutex> lock(MainHelper::GetInstance().LoadedScriptsMutex());
            auto& scripts = MainHelper::GetInstance().LoadedScripts();
            auto& oldEntry = scripts[indexToReload];
            StopScriptExecution(oldEntry.data, true, false);
            ScriptEntry newEntry;
            std::u8string errC, errT;
            if (!LoadScriptEntry(oldEntry.filePath, errC, errT, newEntry)) {
                MsgBox().Show(errC, errT);
            } else {
                newEntry.data.isEnabled = oldEntry.data.isEnabled.load();
                scripts[indexToReload] = std::move(newEntry);
            }
        }
        if (indexToOpenScriptLocation != -1) {
            std::filesystem::path folderPath = std::filesystem::path(s_tableRows[indexToOpenScriptLocation].filePath).parent_path();

            if (std::filesystem::exists(folderPath)) {
                ShellExecuteW(nullptr, L"open", folderPath.wstring().c_str(), nullptr, nullptr, SW_SHOWNORMAL);
            }
        }

        ImGui::EndTable();
    }

#ifdef _LANG_ZH_TW_
    if (ImGui::Button("開啟全部腳本")) {
#else
    if (ImGui::Button("Enable All")) {
#endif
        std::lock_guard<std::mutex> lock(MainHelper::GetInstance().LoadedScriptsMutex());
        for (auto& s : MainHelper::GetInstance().LoadedScripts()) {
            s.data.isEnabled = true;
        }
        for (auto& r : s_tableRows) {
            r.isEnabled = true;
        }
    }
    ImGui::SameLine();
#ifdef _LANG_ZH_TW_
    if (ImGui::Button("關閉全部腳本")) {
#else
    if (ImGui::Button("Disable All)") {
#endif
        std::lock_guard<std::mutex> lock(MainHelper::GetInstance().LoadedScriptsMutex());
        for (auto& s : MainHelper::GetInstance().LoadedScripts()) {
            s.data.isEnabled = false;
            StopScriptExecution(s.data, true, false);
        }
        for (auto& r : s_tableRows) {
            r.isEnabled = false;
        }
    }
    ImGui::SameLine();
#ifdef _LANG_ZH_TW_
    if (ImGui::Button("移除全部腳本")) {
#else
    if (ImGui::Button("Remove All")) {
#endif
        std::lock_guard<std::mutex> lock(MainHelper::GetInstance().LoadedScriptsMutex());
        auto& scripts = MainHelper::GetInstance().LoadedScripts();
        for (auto& s : scripts) {
            StopScriptExecution(s.data, true, false);
        }
        scripts.clear();
        s_tableRows.clear();
    }

    ImGui::PopStyleVar(3);
    ImGui::End();

    MsgBox().Render();

    if (s_showRecordWnd) RecordWnd::RenderWindow(hwnd, &s_showRecordWnd);
    if (s_showFunctionsRefWnd) FunctionsRefWnd::RenderWindow(hwnd, &s_showFunctionsRefWnd);
    if (s_showKeysRefWnd) KeysRefWnd::RenderWindow(hwnd, &s_showKeysRefWnd);
}

ImMessageBox& MainWnd::MsgBox() {
    static ImMessageBox instance;
    return instance;
}
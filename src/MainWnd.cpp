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
        outErrorContent = I18n::FormatU8(StringId::MsgReadScriptFailed, entry.filePath);
        outErrorTitle = I18n::GetU8(StringId::TitleFileError);
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
        outErrorTitle = I18n::GetU8(StringId::TitleScriptError);
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

bool MainWnd::ChangeKeyAndModePopup(
    std::u8string filePath,
    const std::u8string& startKey,
    std::vector<std::u8string> startKeyModifiers,
    const std::u8string& endKey,
    std::vector<std::u8string> endKeyModifiers,
    const std::u8string& runMode
) {
    std::vector<std::u8string> fileLines;
    if (!ReadUtf8FileLines(filePath, fileLines)) {
        MsgBox().Show(I18n::GetU8(StringId::MsgReadFileFailed), I18n::GetU8(StringId::TitleSaveFailed));
        return false;
    }

    std::vector<std::u8string> filteredLines;
    for (const auto& line : fileLines) {
        std::u8string trimmedLine = std::u8string(StringTrim(line));

        bool isHeader = false;
        if (!trimmedLine.empty()) {
            size_t openParen = trimmedLine.find(u8'(');
            size_t closeParen = trimmedLine.rfind(u8')');
            if (openParen != std::u8string::npos && closeParen != std::u8string::npos && closeParen > openParen) {
                std::u8string func = std::u8string(StringTrim(StringLower(std::u8string_view(trimmedLine).substr(0, openParen))));
                if (func == u8"設定開始按鍵" || func == u8"setstartkey" || func == u8"设定开始按键" ||
                    func == u8"設定停止按鍵" || func == u8"setendkey" || func == u8"设定停止按键" ||
                    func == u8"設定運行模式" || func == u8"setrunmode" || func == u8"设定运行模式") {
                    isHeader = true;
                }
            }
        }

        if (!isHeader) {
            filteredLines.push_back(line);
        }
    }

    // Remove leading empty lines left behind
    while (!filteredLines.empty() && std::u8string(StringTrim(filteredLines.front())).empty()) {
        filteredLines.erase(filteredLines.begin());
    }

    // Build new headers
    std::vector<std::u8string> combinedStartKeys;
    combinedStartKeys.reserve(1 + startKeyModifiers.size());
    combinedStartKeys.push_back(startKey);
    combinedStartKeys.insert(combinedStartKeys.end(), startKeyModifiers.begin(), startKeyModifiers.end());

    std::vector<std::u8string> combinedEndKeys;
    combinedEndKeys.reserve(1 + endKeyModifiers.size());
    combinedEndKeys.push_back(endKey);
    combinedEndKeys.insert(combinedEndKeys.end(), endKeyModifiers.begin(), endKeyModifiers.end());

    std::u8string hSetStartKey = u8"SetStartKey";
    std::u8string hSetEndKey = u8"SetEndKey";
    std::u8string hSetRunMode = u8"SetRunMode";
    if (I18n::GetLanguage() == Language::TraditionalChinese || I18n::GetLanguage() == Language::Cantonese) {
        hSetStartKey = u8"設定開始按鍵";
        hSetEndKey = u8"設定停止按鍵";
        hSetRunMode = u8"設定運行模式";
    } else if (I18n::GetLanguage() == Language::SimplifiedChinese) {
        hSetStartKey = u8"设定开始按键";
        hSetEndKey = u8"设定停止按键";
        hSetRunMode = u8"设定运行模式";
    }

    std::vector<std::u8string> headers = {
        hSetStartKey + u8"(" + JoinU8Strings(combinedStartKeys, u8", ") + u8")",
        hSetEndKey + u8"(" + JoinU8Strings(combinedEndKeys, u8", ") + u8")",
        hSetRunMode + u8"(" + runMode + u8")",
        u8"",
    };

    // Prepend headers to filtered lines
    filteredLines.insert(filteredLines.begin(), headers.begin(), headers.end());

    // Write back to file
    std::ofstream file(std::filesystem::path(u8stringToWString(filePath)), std::ios::binary);
    if (!file.is_open()) {
        MsgBox().Show(I18n::GetU8(StringId::MsgOpenFileWriteFailed), I18n::GetU8(StringId::TitleSaveFailed));
        return false;
    }

    try {
        uint8_t bom[] = { 0xEF, 0xBB, 0xBF };
        uint8_t nextLine[] = { '\r', '\n' };
        file.write(reinterpret_cast<char*>(bom), sizeof(bom));

        for (const auto& line : filteredLines) {
            file.write(reinterpret_cast<const char*>(line.c_str()), static_cast<std::streamsize>(line.length()));
            file.write(reinterpret_cast<const char*>(nextLine), sizeof(nextLine));
        }
    }
    catch (...) {
        file.close();
        MsgBox().Show(I18n::GetU8(StringId::MsgWriteFileUnexpected), I18n::GetU8(StringId::TitleSaveFailed));
        return false;
    }
    file.close();
    return true;
}

void MainWnd::RenderWindow(HWND hwnd) {
    ImGuiViewport* viewport = ImGui::GetMainViewport();

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


    ImGui::Begin(I18n::Get(StringId::MainWndTitle), nullptr, window_flags);

    if (ImGui::Button(I18n::Get(StringId::OpenScripts))) {
        std::vector<std::u8string> filePaths;
        if (OpenScriptFilesDialog(hwnd, filePaths)) {
            ProcessScriptFiles(filePaths);
        }
    }

    ImGui::SameLine();
    if (ImGui::Button(I18n::Get(StringId::ScriptRecorder))) s_showRecordWnd = true;
    ImGui::SameLine();
    if (ImGui::Button(I18n::Get(StringId::FunctionsRef))) s_showFunctionsRefWnd = true;
    ImGui::SameLine();
    if (ImGui::Button(I18n::Get(StringId::KeysRef))) s_showKeysRefWnd = true;

    ImGui::Separator();

    ImGuiTableFlags tableFlags = ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg |
        ImGuiTableFlags_Resizable | ImGuiTableFlags_ScrollY; // |
        //ImGuiTableFlags_HighlightHoveredColumn;

    bool showChangeKeyAndMode = false;
    if (ImGui::BeginTable("ScriptTable", 5, tableFlags, ImVec2(0, -ImGui::GetFrameHeightWithSpacing()))) {
        ImGui::TableSetupColumn(I18n::Get(StringId::ColEnable), ImGuiTableColumnFlags_WidthFixed, 60.0f);
        ImGui::TableSetupColumn(I18n::Get(StringId::ColScriptName), ImGuiTableColumnFlags_WidthStretch, 2.0f);
        ImGui::TableSetupColumn(I18n::Get(StringId::ColExecutionMode), ImGuiTableColumnFlags_WidthFixed, 120.0f);
        ImGui::TableSetupColumn(I18n::Get(StringId::ColStartKey), ImGuiTableColumnFlags_WidthFixed, 120.0f);
        ImGui::TableSetupColumn(I18n::Get(StringId::ColEndKey), ImGuiTableColumnFlags_WidthFixed, 120.0f);
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
                    if (ImGui::MenuItem(I18n::Get(StringId::MenuDisable))) {
                        std::lock_guard<std::mutex> lock(MainHelper::GetInstance().LoadedScriptsMutex());
                        auto& entry = MainHelper::GetInstance().LoadedScripts().at(i);
                        entry.data.isEnabled = false;
                        row.isEnabled = false;
                        StopScriptExecution(entry.data, true, false); 
                    }
                } else {
                    if (ImGui::MenuItem(I18n::Get(StringId::MenuEnable))) {
                        std::lock_guard<std::mutex> lock(MainHelper::GetInstance().LoadedScriptsMutex());
                        auto& entry = MainHelper::GetInstance().LoadedScripts().at(i);
                        entry.data.isEnabled = true;
                        row.isEnabled = true;
                    }
                }
                ImGui::Separator();
                if (s_testScript.isRunning) {
                    if (ImGui::MenuItem(I18n::Get(StringId::MenuStopTestRun))) {
                        StopScriptExecution(s_testScript, true, false);
                    }
                }
                else {
                    if (ImGui::MenuItem(I18n::Get(StringId::MenuRunTest))) {
                        std::lock_guard<std::mutex> lock(MainHelper::GetInstance().LoadedScriptsMutex());
                        auto& entry = MainHelper::GetInstance().LoadedScripts().at(i);
                        s_testScript.commands = entry.data.commands;
                        s_testScript.mode = ScriptRunMode::FullSingle;
                        s_testScript.isEnabled = true;
                        StartScriptExecution(s_testScript);
                    }
                }
                ImGui::Separator();
                if (ImGui::MenuItem(I18n::Get(StringId::MenuRemove))) {
                    indexToRemove = static_cast<int>(i); 
                }
                if (ImGui::MenuItem(I18n::Get(StringId::MenuReload))) {
                    indexToReload = static_cast<int>(i); 
                }
                ImGui::Separator();
                if (ImGui::MenuItem(I18n::Get(StringId::MenuOpenScriptLocation))) {
                    indexToOpenScriptLocation = static_cast<int>(i);
                }
                if (ImGui::MenuItem(I18n::Get(StringId::MenuChangeKeysAndMode))) {
                    s_indexToChangeKeyAndMode = static_cast<int>(i);
                    showChangeKeyAndMode = true;
                }
                ImGui::EndPopup();
            }

            ImGui::SameLine();
            ImGui::SetCursorPosX(ImGui::GetCursorPosX() - ImGui::GetStyle().ItemSpacing.x);
            ImGui::Text("%s", reinterpret_cast<const char*>(row.fileName.c_str()));

            ImGui::TableSetColumnIndex(2);
            ImGui::Text("%s", reinterpret_cast<const char*>(row.displayMode.c_str()));

            ImGui::TableSetColumnIndex(3);
            ImGui::Text("%s", reinterpret_cast<const char*>(row.displayStartKey.c_str()));

            ImGui::TableSetColumnIndex(4);
            ImGui::Text("%s", reinterpret_cast<const char*>(row.displayEndKey.c_str()));

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
                s_tableRows[indexToReload].mode = newEntry.data.mode;
                s_tableRows[indexToReload].startKey = newEntry.data.startKey;
                s_tableRows[indexToReload].endKey = newEntry.data.endKey;
                s_tableRows[indexToReload].displayMode = ScriptRunModeToString(newEntry.data.mode);
                s_tableRows[indexToReload].displayStartKey = HotkeyDataToString(newEntry.data.startKey);
                s_tableRows[indexToReload].displayEndKey = HotkeyDataToString(newEntry.data.endKey);
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

    if (ImGui::Button(I18n::Get(StringId::EnableAll))) {
        std::lock_guard<std::mutex> lock(MainHelper::GetInstance().LoadedScriptsMutex());
        for (auto& s : MainHelper::GetInstance().LoadedScripts()) {
            s.data.isEnabled = true;
        }
        for (auto& r : s_tableRows) {
            r.isEnabled = true;
        }
    }
    ImGui::SameLine();
    if (ImGui::Button(I18n::Get(StringId::DisableAll))) {
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
    if (ImGui::Button(I18n::Get(StringId::RemoveAll))) {
        std::lock_guard<std::mutex> lock(MainHelper::GetInstance().LoadedScriptsMutex());
        auto& scripts = MainHelper::GetInstance().LoadedScripts();
        for (auto& s : scripts) {
            StopScriptExecution(s.data, true, false);
        }
        scripts.clear();
        s_tableRows.clear();
    }

    ImGui::SameLine();
    float langComboWidth = 135.0f;
    float availWidth = ImGui::GetContentRegionAvail().x;
    if (availWidth > langComboWidth) {
        ImGui::SetCursorPosX(ImGui::GetCursorPosX() + availWidth - langComboWidth);
    }
    ImGui::SetNextItemWidth(langComboWidth);
    int currentLang = static_cast<int>(I18n::GetLanguage());
    if (ImGui::Combo("##LanguageSelect", &currentLang, I18n::GetLanguageNames(), static_cast<int>(Language::Count))) {
        I18n::SetLanguage(static_cast<Language>(currentLang));
        for (auto& row : s_tableRows) {
            row.displayMode = ScriptRunModeToString(row.mode);
        }
        I18n::SaveConfig();
    }

    ImGui::PopStyleVar(3);
    ImGui::End();

    if (showChangeKeyAndMode) {
        s_changeKeyAndModePopUp.Open(I18n::Get(StringId::MenuChangeKeysAndMode), s_tableRows[s_indexToChangeKeyAndMode].mode, s_tableRows[s_indexToChangeKeyAndMode].startKey, s_tableRows[s_indexToChangeKeyAndMode].endKey);
    }

    if (s_indexToChangeKeyAndMode != -1) {
        s_changeKeyAndModePopUp.RenderPopup(viewport, NULL, [&](
            std::u8string startKey,
            std::vector<std::u8string> startKeyModifiers,
            std::u8string endKey,
            std::vector<std::u8string> endKeyModifiers,
            std::u8string runMode
            ) {
                std::lock_guard<std::mutex> lock(MainHelper::GetInstance().LoadedScriptsMutex());
                auto& scripts = MainHelper::GetInstance().LoadedScripts();
                auto& oldEntry = scripts[s_indexToChangeKeyAndMode];
                std::u8string filePath = oldEntry.filePath;
                if (ChangeKeyAndModePopup(filePath, startKey, startKeyModifiers, endKey, endKeyModifiers, runMode)) {
                    StopScriptExecution(oldEntry.data, true, false);
                    ScriptEntry newEntry;
                    std::u8string errC, errT;
                    if (!LoadScriptEntry(filePath, errC, errT, newEntry)) {
                        MsgBox().Show(errC, errT);
                    }
                    else {
                        newEntry.data.isEnabled = oldEntry.data.isEnabled.load();
                        s_tableRows[s_indexToChangeKeyAndMode].mode = newEntry.data.mode;
                        s_tableRows[s_indexToChangeKeyAndMode].startKey = newEntry.data.startKey;
                        s_tableRows[s_indexToChangeKeyAndMode].endKey = newEntry.data.endKey;
                        s_tableRows[s_indexToChangeKeyAndMode].displayMode = ScriptRunModeToString(newEntry.data.mode);
                        s_tableRows[s_indexToChangeKeyAndMode].displayStartKey = HotkeyDataToString(newEntry.data.startKey);
                        s_tableRows[s_indexToChangeKeyAndMode].displayEndKey = HotkeyDataToString(newEntry.data.endKey);
                        scripts[s_indexToChangeKeyAndMode] = std::move(newEntry);
                        MsgBox().Show(I18n::FormatW(StringId::MsgScriptSettingsUpdated, u8stringToWString(filePath)), I18n::GetW(StringId::TitleSaveSuccess));
                    }
                }
                s_indexToChangeKeyAndMode = -1;
            });
    }

    MsgBox().Render();

    if (s_showRecordWnd) RecordWnd::RenderWindow(hwnd, &s_showRecordWnd);
    if (s_showFunctionsRefWnd) FunctionsRefWnd::RenderWindow(hwnd, &s_showFunctionsRefWnd);
    if (s_showKeysRefWnd) KeysRefWnd::RenderWindow(hwnd, &s_showKeysRefWnd);
}

ImMessageBox& MainWnd::MsgBox() {
    static ImMessageBox instance;
    return instance;
}

void MainWnd::ProcessScriptFiles(const std::vector<std::u8string>& filePaths) {
    for (const auto& path : filePaths) {
        ScriptEntry entry;
        std::u8string errContent, errTitle;

        if (!LoadScriptEntry(path, errContent, errTitle, entry)) {
            MsgBox().Show(errContent, errTitle);
            break;
        }

        s_tableRows.push_back({
            .isEnabled = false,
            .filePath = entry.filePath,
            .fileName = entry.fileName,
            .mode = entry.data.mode,
            .startKey = entry.data.startKey,
            .endKey = entry.data.endKey,
            .displayMode = ScriptRunModeToString(entry.data.mode),
            .displayStartKey = HotkeyDataToString(entry.data.startKey),
            .displayEndKey = HotkeyDataToString(entry.data.endKey),
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
            catch (const std::filesystem::filesystem_error&) {}
        }
        scripts.push_back(std::move(entry));
    }
}

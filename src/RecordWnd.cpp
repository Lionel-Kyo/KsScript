#include "pch.h"
#include "RecordWnd.h"

static std::u8string g_scriptTextBuffer = u8"";
static bool g_needsInitialSync = true;

void RecordWnd::SyncEditToBuffer() {
    std::string charBuffer(reinterpret_cast<const char*>(g_scriptTextBuffer.data()), g_scriptTextBuffer.size());
    std::stringstream ss(charBuffer);

    std::u8string line;
    std::vector<std::u8string> buffer;

    while (u8getline(ss, line)) {
        if (!line.empty() && line.back() == u8'\r') line.pop_back();
        if (!line.empty()) {
            buffer.push_back(line);
        }
    }
    RecordHelper::GetInstance().SetScriptBuffer(buffer);
}

void RecordWnd::SyncBufferToEdit() {
    std::vector<std::u8string> buffer;
    RecordHelper::GetInstance().GetScriptBuffer(buffer);

    g_scriptTextBuffer.clear();
    for (const auto& line : buffer) {
        g_scriptTextBuffer += line + u8"\r\n";
    }
}

bool RecordWnd::SaveScriptToFile(HWND hwnd) {
    SyncEditToBuffer();
    std::vector<std::u8string> lines;
    RecordHelper::GetInstance().GetScriptBuffer(lines);

#ifdef _LANG_ZH_TW_
    std::vector<std::u8string> headers = {
        u8"設定開始按鍵(F12)",
        u8"設定停止按鍵(None)",
        u8"設定運行模式(完整單次)",
        u8"",
    };
#else
    std::vector<std::u8string> headers = {
        u8"SetStartKey(F12)",
        u8"SetEndKey(None)",
        u8"SetRunMode(FullSingle)",
        u8"",
    };
#endif

    constexpr size_t LongPathMax = 32768;
    std::vector<wchar_t> szFile(LongPathMax, L'\0');

    OPENFILENAMEW ofn = { sizeof(ofn) };
    ofn.hwndOwner = hwnd;
    ofn.lpstrFilter = L"UTF-8 Text Scripts (*.txt)\0*.txt\0All Files (*.*)\0*.*\0";
    ofn.lpstrFile = szFile.data();
    ofn.nMaxFile = (DWORD)LongPathMax;
    ofn.Flags = OFN_OVERWRITEPROMPT | OFN_HIDEREADONLY | OFN_LONGNAMES;
    ofn.lpstrDefExt = L"txt";

    if (GetSaveFileNameW(&ofn)) {
        std::wstring filePath = szFile.data();
        std::ofstream file(std::filesystem::path(filePath), std::ios::binary);
        if (!file.is_open()) {
#ifdef _LANG_ZH_TW_
            MsgBox().Show(u8"無法開啟檔案以進行寫入", u8"保存失敗");
#else
            MsgBox().Show(u8"Failed to open file for writing", u8"Save File Failed");
#endif
            return false;
        }

        try {
            uint8_t bom[] = { 0xEF, 0xBB, 0xBF };
            uint8_t nextLine[] = { '\r', '\n' };
            file.write(reinterpret_cast<char*>(bom), sizeof(bom));

            auto combined = std::array{ std::ref(headers), std::ref(lines) };
            for (const auto& containerRef : combined) {
                for (const auto& line : containerRef.get()) {
                    file.write(reinterpret_cast<const char*>(line.c_str()), static_cast<std::streamsize>(line.length()));
                    file.write(reinterpret_cast<const char*>(nextLine), sizeof(nextLine));
                }
            }
        } catch (...) {
            file.close();
            return false;
        }
        file.close();

#ifdef _LANG_ZH_TW_
        MsgBox().Show(std::format(L"腳本已保存至: {}", filePath), L"保存成功");
#else
        MsgBox().Show(std::format(L"Script is saved to: {}", filePath), L"Save Success");
#endif
        return true;
    }
    return false;
}

void RecordWnd::RenderWindow(HWND hwnd, bool* pOpen) {
    if (g_needsInitialSync) {
        SyncBufferToEdit();
        g_needsInitialSync = false;
    }

    ImGuiViewport* viewport = ImGui::GetMainViewport();

    ImVec2 windowSize(500, 400);
    ImVec2 windowPos(viewport->GetCenter().x - windowSize.x * 0.5f, viewport->GetCenter().y - windowSize.y * 0.5f);

    ImGui::SetNextWindowSize(windowSize, ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowPos(windowPos, ImGuiCond_FirstUseEver);

#ifdef _LANG_ZH_TW_
    if (!ImGui::Begin("腳本記錄器", pOpen)) {
        ImGui::End();
        return;
    }
#else
    if (!ImGui::Begin("Script Recorder", pOpen)) {
        ImGui::End();
        return;
    }
#endif

    if (ImGui::IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows) &&
        ImGui::IsKeyPressed(ImGuiKey_Escape)) {
        *pOpen = false;
    }

    bool isRecording = RecordHelper::GetInstance().IsRecording();
    bool isPlaying = RecordHelper::GetInstance().IsPlaying();

    if (isPlaying) ImGui::BeginDisabled();
#ifdef _LANG_ZH_TW_
    if (ImGui::Button(isRecording ? "停止記錄" : "記錄")) {
#else
    if (ImGui::Button(isRecording ? "Stop Rec" : "Record")) {
#endif
        if (isRecording) {
            RecordHelper::GetInstance().StopRecording();
            SyncBufferToEdit();
        } else {
            SyncEditToBuffer();
            RecordHelper::GetInstance().StartRecording();
        }
    }
    if (isPlaying) ImGui::EndDisabled();

    ImGui::SameLine();

    if (isRecording) ImGui::BeginDisabled();
#ifdef _LANG_ZH_TW_
    if (ImGui::Button(isPlaying ? "停止執行" : "執行")) {
#else
    if (ImGui::Button(isPlaying ? "Stop Play" : "Play")) {
#endif
        if (isPlaying) {
            RecordHelper::GetInstance().StopPlayback();
        } else {
            SyncEditToBuffer();
            std::vector<ParsedCommand> parsedCommands;
            std::u8string errorMsg;
            int errorLine = 0;
            if (!RecordHelper::GetInstance().GetCommands(parsedCommands, errorMsg, errorLine)) {
#ifdef _LANG_ZH_TW_
                MsgBox().Show(errorMsg, u8"腳本錯誤");
#else
                MsgBox().Show(errorMsg, u8"Script Syntax Error");
#endif
            } else {
                RecordHelper::GetInstance().StartPlayback(parsedCommands, nullptr, nullptr);
            }
        }
    }
    if (isRecording) ImGui::EndDisabled();

    ImGui::SameLine();

#ifdef _LANG_ZH_TW_
    if (ImGui::Button("保存腳本")) {
#else
    if (ImGui::Button("Save Script"))) {
#endif
        SaveScriptToFile(hwnd);
    }

#ifdef _LANG_ZH_TW_
    if (isRecording) ImGui::TextColored(ImVec4(1, 0, 0, 1), "狀態: 記錄中...");
    else if (isPlaying) ImGui::TextColored(ImVec4(0, 1, 0, 1), "狀態: 執行中...");
    else ImGui::Text("狀態: 無");
#else
    if (isRecording) ImGui::TextColored(ImVec4(1, 0, 0, 1), "Status: RECORDING...");
    else if (isPlaying) ImGui::TextColored(ImVec4(0, 1, 0, 1), "Status: EXECUTING...");
    else ImGui::Text("Status: IDLE");
#endif

    ImGui::Separator();

    ImVec2 availableSize = ImGui::GetContentRegionAvail();
    static std::vector<char> textBuffer;
    textBuffer.resize(g_scriptTextBuffer.size() + 1);
    std::memcpy(textBuffer.data(), g_scriptTextBuffer.data(), g_scriptTextBuffer.size());
    textBuffer[g_scriptTextBuffer.size()] = '\0';

    if (ImGui::InputTextMultiline("##ScriptEdit", textBuffer.data(), textBuffer.size(), availableSize, ImGuiInputTextFlags_AllowTabInput)) {
        g_scriptTextBuffer = reinterpret_cast<const char8_t*>(textBuffer.data());
    }


    ImGui::End();

    MsgBox().Render();
}

ImMessageBox& RecordWnd::MsgBox() {
    static ImMessageBox instance;
    return instance;
}
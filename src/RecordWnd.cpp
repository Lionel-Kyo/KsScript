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

bool RecordWnd::SaveScriptToFile(
    HWND hwnd, 
    const std::u8string& startKey, 
    std::vector<std::u8string> startKeyModifiers, 
    const std::u8string& endKey,
    std::vector<std::u8string> endKeyModifiers,
    const std::u8string& runMode
) {
    SyncEditToBuffer();
    std::vector<std::u8string> lines;
    RecordHelper::GetInstance().GetScriptBuffer(lines);

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
            MsgBox().Show(I18n::GetU8(StringId::MsgOpenFileWriteFailed), I18n::GetU8(StringId::TitleSaveFailed));
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

        MsgBox().Show(I18n::FormatW(StringId::MsgScriptSavedTo, filePath), I18n::GetW(StringId::TitleSaveSuccess));
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

    if (!ImGui::Begin(I18n::Get(StringId::ScriptRecorder), pOpen)) {
        ImGui::End();
        return;
    }

    if (ImGui::IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows) && ImGui::IsKeyPressed(ImGuiKey_Escape)) {
        *pOpen = false;
    }

    bool isRecording = RecordHelper::GetInstance().IsRecording();
    bool isPlaying = RecordHelper::GetInstance().IsPlaying();

    if (isPlaying) ImGui::BeginDisabled();
    if (ImGui::Button(I18n::Get(isRecording ? StringId::StopRec : StringId::Record))) {
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
    if (ImGui::Button(I18n::Get(isPlaying ? StringId::StopPlay : StringId::Play))) {
        if (isPlaying) {
            RecordHelper::GetInstance().StopPlayback();
        } else {
            SyncEditToBuffer();
            std::vector<ParsedCommand> parsedCommands;
            std::u8string errorMsg;
            int errorLine = 0;
            if (!RecordHelper::GetInstance().GetCommands(parsedCommands, errorMsg, errorLine)) {
                MsgBox().Show(errorMsg, I18n::GetU8(StringId::TitleScriptError));
            } else {
                RecordHelper::GetInstance().StartPlayback(parsedCommands, nullptr, nullptr);
            }
        }
    }
    if (isRecording) ImGui::EndDisabled();

    ImGui::SameLine();

    if (ImGui::Button(I18n::Get(StringId::SaveScript))) {
        s_changeKeyAndModePopUp.Open(I18n::Get(StringId::SaveScript), std::nullopt, std::nullopt, std::nullopt);
    }

    s_changeKeyAndModePopUp.RenderPopup(viewport, NULL, [&](
        std::u8string startKey,
        std::vector<std::u8string> startKeyModifiers,
        std::u8string endKey,
        std::vector<std::u8string> endKeyModifiers,
        std::u8string runMode
    ) {
        SaveScriptToFile(hwnd, startKey, startKeyModifiers, endKey, endKeyModifiers, runMode);
    });

    if (isRecording) ImGui::TextColored(ImVec4(1, 0, 0, 1), "%s", I18n::Get(StringId::StatusRecording));
    else if (isPlaying) ImGui::TextColored(ImVec4(0, 1, 0, 1), "%s", I18n::Get(StringId::StatusPlaying));
    else ImGui::Text("%s", I18n::Get(StringId::StatusIdle));

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
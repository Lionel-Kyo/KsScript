#pragma once
#ifndef _MAIN_WND_H_
#define _MAIN_WND_H_

#include <string>
#include <vector>
#include <filesystem>
#include <windows.h>
#include <commdlg.h>
#include "MainHelper.h"
#include "Utils.h"
#include "FunctionsRefWnd.h"
#include "RecordWnd.h"
#include "KeysRefWnd.h"
#include "ImMessageBox.h"
#include "XorString.h"
#include "libs/ImGui/imgui.h"

struct MainTableRow {
    bool isEnabled;
    std::u8string filePath;
    std::u8string fileName;
    ScriptRunMode mode;
    HotkeyData startKey;
    HotkeyData endKey;
    std::u8string displayMode;
    std::u8string displayStartKey;
    std::u8string displayEndKey;
};

class MainWnd {
public:
    static void RenderWindow(HWND hwnd);
    static ImMessageBox& MsgBox();
    static void ProcessScriptFiles(const std::vector<std::u8string>& filePaths);

private:
    inline static std::vector<MainTableRow> s_tableRows = {};
    inline static bool s_showRecordWnd = false;
    inline static bool s_showFunctionsRefWnd = false;
    inline static bool s_showKeysRefWnd = false;
    inline static int s_indexToChangeKeyAndMode = -1;
    inline static ScriptData s_testScript{};
    inline static ChangeKeyAndModePopUp s_changeKeyAndModePopUp{};
    static bool OpenScriptFilesDialog(HWND hwnd, std::vector<std::u8string>& outPaths);
    static bool LoadScriptEntry(const std::u8string& filePath, std::u8string& outErrorContent, std::u8string& outErrorTitle, ScriptEntry& outEntry);
    static bool ChangeKeyAndModePopup(std::u8string filePath, const std::u8string& startKey, std::vector<std::u8string> startKeyModifiers, const std::u8string& endKey, std::vector<std::u8string> endKeyModifiers, const std::u8string& runMode);
};

#endif // !_MAIN_WND_H_
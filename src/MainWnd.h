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
    std::u8string mode;
    std::u8string startKey;
    std::u8string endKey;
};

class MainWnd {
public:
    static void RenderWindow(HWND hwnd);
    static ImMessageBox& MsgBox();

private:
    inline static std::vector<MainTableRow> s_tableRows = {};
    inline static bool s_showRecordWnd = false;
    inline static bool s_showFunctionsRefWnd = false;
    inline static bool s_showKeysRefWnd = false;
    inline static ScriptData s_testScript{};
    static bool OpenScriptFilesDialog(HWND hwnd, std::vector<std::u8string>& outPaths);
    static bool LoadScriptEntry(const std::u8string& filePath, std::u8string& outErrorContent, std::u8string& outErrorTitle, ScriptEntry& outEntry);
};

#endif // !_MAIN_WND_H_
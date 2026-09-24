#pragma once
#ifndef _RECORD_WND_H_
#define _RECORD_WND_H_

#include <string>
#include <vector>
#include <functional>
#include <windows.h>
#include <sstream>
#include <fstream>
#include <array>
#include <format>
#include <commdlg.h>
#include "libs/ImGui/imgui.h"
#include "ImMessageBox.h"
#include "RecordHelper.h" 
#include "XorString.h"

class RecordWnd {
public:
    static void RenderWindow(HWND hwnd, bool* pOpen);
    static ImMessageBox& MsgBox();

private:
    static void SyncEditToBuffer();
    static void SyncBufferToEdit();
    static bool SaveScriptToFile(HWND hwnd);
};

#endif // !_RECORD_WND_H_

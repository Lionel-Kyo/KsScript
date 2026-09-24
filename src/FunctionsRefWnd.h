#pragma once
#ifndef _FUNCTIONS_REF_WND_H_
#define _FUNCTIONS_REF_WND_H_

#include <windows.h>

#include "libs/ImGui/imgui.h"
#include "Utils.h"
#include "XorString.h"

struct FuncInfo {
    std::u8string name;
    std::u8string parameters;
    std::u8string explain;
};

class FunctionsRefWnd {
public:
    static void RenderWindow(HWND hwnd, bool* pOpen);
};

#endif // !_FUNCTIONS_REF_WND_H_
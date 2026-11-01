#pragma once
#ifndef _FUNCTIONS_REF_WND_H_
#define _FUNCTIONS_REF_WND_H_

#include <windows.h>

#include "libs/ImGui/imgui.h"
#include "Utils.h"
#include "XorString.h"



class FunctionsRefWnd {
public:
    static void RenderWindow(HWND hwnd, bool* pOpen);
};

#endif // !_FUNCTIONS_REF_WND_H_
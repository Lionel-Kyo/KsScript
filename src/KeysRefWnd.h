#pragma once
#ifndef _KEYS_REF_WND_H_
#define _KEYS_REF_WND_H_

#include <windows.h>
#include <string>
#include <vector>
#include <utility>
#include <algorithm>
#include <format>

#include "libs/ImGui/imgui.h"
#include "Utils.h"
#include "XorString.h"

class KeysRefWnd {
public:
    static void RenderWindow(HWND hwnd, bool* pOpen);
};

#endif // !_KEYS_REF_WND_H_
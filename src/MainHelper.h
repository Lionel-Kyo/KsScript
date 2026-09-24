#pragma once
#ifndef _MAIN_HELPER_H_
#define _MAIN_HELPER_H_

#include <Windows.h>
#include <thread>
#include <mutex>
#include <vector>
#include <string>
#include <sstream>
#include <atomic>
#include <ranges>
#include <algorithm>
#include <format>
#include <functional>
#include <unordered_map>
#include "Utils.h"

class MainHelper {
public:
    static MainHelper& GetInstance();
    void SetHWnd(HWND hwnd);
    HWND GetHWnd();
    void StartKeyboardHook();
    void StopKeyboardHook();
    std::vector<ScriptEntry>& LoadedScripts();
    std::mutex& LoadedScriptsMutex();
private:
    HWND m_hWnd = nullptr;
    HHOOK m_keyboardHook = nullptr;
    std::unordered_map<int32_t, bool> m_keyboardDown;
    std::vector<ScriptEntry> m_loadedScripts;
    std::mutex m_loadedScriptsMutex;
    std::jthread m_handleKeyboardEventThread;
    DWORD m_handleKeyboardEventThreadId = 0;
    static void KeyboardEventWorker(std::stop_token stopToken);
    static LRESULT LowLevelKeyboardProc(int nCode, WPARAM wParam, LPARAM lParam);
};

#endif // !_MAIN_HELPER_H_
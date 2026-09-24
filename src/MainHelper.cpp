#include "pch.h"
#include "MainHelper.h"
#include "res/Resource.h"

MainHelper& MainHelper::GetInstance() {
    static MainHelper instance;
    return instance;
}

void MainHelper::SetHWnd(HWND hwnd) {
    m_hWnd = hwnd;
}

HWND MainHelper::GetHWnd() {
    return m_hWnd;
}

void MainHelper::StartKeyboardHook() {
    m_handleKeyboardEventThread = std::jthread(KeyboardEventWorker);
    m_keyboardHook = SetWindowsHookExW(WH_KEYBOARD_LL, MainHelper::LowLevelKeyboardProc, GetModuleHandle(nullptr), 0);
}

void MainHelper::StopKeyboardHook() {
    if (m_keyboardHook) UnhookWindowsHookEx(m_keyboardHook);
    m_keyboardHook = nullptr;
    if (m_handleKeyboardEventThread.joinable()) {
        m_handleKeyboardEventThread.request_stop();
        if (m_handleKeyboardEventThreadId != 0) {
            ::PostThreadMessage(m_handleKeyboardEventThreadId, WM_ON_KEYBOARD_EVENT_EXIT, 0, 0);
        }
        m_handleKeyboardEventThread.join();
    }
}

std::vector<ScriptEntry>& MainHelper::LoadedScripts() {
    return m_loadedScripts;
}

std::mutex& MainHelper::LoadedScriptsMutex() {
    return m_loadedScriptsMutex;
}


void MainHelper::KeyboardEventWorker(std::stop_token stopToken) {
    auto& instance = MainHelper::GetInstance();
    instance.m_handleKeyboardEventThreadId = GetCurrentThreadId();

    MSG msg{};
    PeekMessage(&msg, nullptr, WM_USER, WM_USER, PM_NOREMOVE);

    while (!stopToken.stop_requested()) {
        const BOOL result = GetMessage(&msg, nullptr, 0, 0);

        if (result <= 0) break;

        if (msg.message == WM_ON_KEYBOARD_EVENT_EXIT) break;

        if (msg.message == WM_ON_KEYBOARD_EVENT) {
            std::unique_ptr<KeyboardEventData> data{ reinterpret_cast<KeyboardEventData*>(msg.lParam) };
            if (data->isDown) {
                std::lock_guard<std::mutex> lock(instance.LoadedScriptsMutex());
                for (auto& entry : instance.LoadedScripts()) {
                    if (!entry.data.isEnabled) continue;
                    // Match end key first, avoid immediate check after started script when start and end is same.
                    if (CheckIsHotkeyDown(entry.data.endKey, *data) && (entry.data.mode == ScriptRunMode::Switch || entry.data.mode == ScriptRunMode::FullSwitch) && entry.data.isRunning) {
                        StopScriptExecution(entry.data, false, false);
                    } else if (CheckIsHotkeyDown(entry.data.startKey, *data)) {
                        if (!entry.data.isRunning) StartScriptExecution(entry.data);
                    }
                }
            } else {
                std::lock_guard<std::mutex> lock(instance.LoadedScriptsMutex());
                for (auto& entry : instance.LoadedScripts()) {
                    if (!entry.data.isEnabled) continue;
                    if (CheckIsHotkeyDown(entry.data.endKey, *data) && (entry.data.mode == ScriptRunMode::Continuous || entry.data.mode == ScriptRunMode::FullContinuous) && entry.data.isRunning) {
                        StopScriptExecution(entry.data, false, false);
                    }
                }
            }
        }
    }
    instance.m_handleKeyboardEventThreadId = 0;
}

LRESULT CALLBACK MainHelper::LowLevelKeyboardProc(int nCode, WPARAM wParam, LPARAM lParam) {
    auto& instance = MainHelper::GetInstance();
    if (nCode >= 0) {
        auto* kbd = reinterpret_cast<KBDLLHOOKSTRUCT*>(lParam);
        if (!(kbd->flags & LLKHF_INJECTED)) {
            auto lastKeyDown = instance.m_keyboardDown.find(kbd->vkCode);
            std::optional<bool> isDown;
            if (wParam == WM_KEYDOWN || wParam == WM_SYSKEYDOWN) {
                // Skip same key down
                if (lastKeyDown == instance.m_keyboardDown.end() || !lastKeyDown->second) {
                    isDown = true;
                    instance.m_keyboardDown[kbd->vkCode] = true;
                }
            }
            else if (wParam == WM_KEYUP || wParam == WM_SYSKEYUP) {
                // Skip same key up
                if (lastKeyDown == instance.m_keyboardDown.end() || lastKeyDown->second) {
                    isDown = false;
                    instance.m_keyboardDown[kbd->vkCode] = false;
                }
            }

            if (isDown.has_value()) {
                auto isKeyDown = [&instance](int32_t vkCode) {
                    auto kv = instance.m_keyboardDown.find(vkCode);
                    if (kv == instance.m_keyboardDown.end()) return false;
                    return kv->second;
                };
                auto* data = new KeyboardEventData{
                    .vkCode = (int32_t)kbd->vkCode,
                    .isDown = isDown.value(),
                    .isLCtrlDown = isKeyDown(VK_LCONTROL),
                    .isRCtrlDown = isKeyDown(VK_RCONTROL),
                    .isLShiftDown = isKeyDown(VK_LSHIFT),
                    .isRShiftDown = isKeyDown(VK_RSHIFT),
                    .isLAltDown = isKeyDown(VK_LMENU),
                    .isRAltDown = isKeyDown(VK_RMENU),
                };
                if (instance.m_handleKeyboardEventThreadId != 0) {
                    ::PostThreadMessage(instance.m_handleKeyboardEventThreadId, WM_ON_KEYBOARD_EVENT, 0, reinterpret_cast<LPARAM>(data));
                }
            }
        }
    }
    return ::CallNextHookEx(instance.m_keyboardHook, nCode, wParam, lParam);
}
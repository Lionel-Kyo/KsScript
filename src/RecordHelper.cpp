#include "pch.h"
#include "RecordHelper.h"

RecordHelper& RecordHelper::GetInstance() {
    static RecordHelper instance;
    return instance;
}

void RecordHelper::RecordCmd(const std::wstring& cmd) {
    auto now = std::chrono::steady_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - m_lastRecordTime).count();
    m_lastRecordTime = now;

    std::lock_guard lock(m_scriptMutex);
    if (elapsed > 10) {
#ifdef _LANG_ZH_TW_
        m_scriptBuffer.push_back(std::format(L"延遲({})", elapsed));
#else
        m_scriptBuffer.push_back(std::format(L"Delay({})", elapsed));
#endif // _LANG_ZH_TW
    }
    m_scriptBuffer.push_back(cmd);
}

LRESULT CALLBACK RecordHelper::LowLevelKeyboardProc(int nCode, WPARAM wParam, LPARAM lParam) {
    auto& instance = RecordHelper::GetInstance();
    if (nCode >= 0 && instance.m_isRecording) {
        auto* kbd = reinterpret_cast<KBDLLHOOKSTRUCT*>(lParam);
        if (!(kbd->flags & LLKHF_INJECTED)) {
            auto keyName = GetNameByKey(kbd->vkCode);
            std::wstring fnName;
            if (wParam == WM_KEYDOWN || wParam == WM_SYSKEYDOWN) {
#ifdef _LANG_ZH_TW_
                fnName = L"鍵盤按下";
#else
                fnName = L"KeyboardDown";
#endif // _LANG_ZH_TW_
            } else if (wParam == WM_KEYUP || wParam == WM_SYSKEYUP) {
#ifdef _LANG_ZH_TW_
                fnName = L"鍵盤放開";
#else
                fnName = L"KeyboardUp";
#endif // _LANG_ZH_TW_
            }
            if (!fnName.empty()) {
                if (keyName.has_value()) {
                    instance.RecordCmd(std::format(L"{}({})", fnName, keyName.value()));
                }
                else {
                    instance.RecordCmd(std::format(L"{}(0x{:X})", fnName, kbd->vkCode));
                }
            }
        }
    }
    return ::CallNextHookEx(instance.m_keyboardHook, nCode, wParam, lParam);
}

LRESULT CALLBACK RecordHelper::LowLevelMouseProc(int nCode, WPARAM wParam, LPARAM lParam) {
    auto& instance = RecordHelper::GetInstance();
    if (nCode >= 0 && instance.m_isRecording) {
        auto* mouse = reinterpret_cast<MSLLHOOKSTRUCT*>(lParam);
        if (!(mouse->flags & LLMHF_INJECTED)) {
            switch (wParam) {
            case WM_LBUTTONDOWN: 
#ifdef _LANG_ZH_TW_
                instance.RecordCmd(L"滑鼠按下(左)"); break;
#else
                instance.RecordCmd(L"MouseDown(Left)"); break;
#endif // _LANG_ZH_TW_
            case WM_LBUTTONUP:   
#ifdef _LANG_ZH_TW_
                instance.RecordCmd(L"滑鼠放開(左)"); break;
#else
                instance.RecordCmd(L"MouseUp(Left)"); break;
#endif // _LANG_ZH_TW_
            case WM_RBUTTONDOWN: 
#ifdef _LANG_ZH_TW_
                instance.RecordCmd(L"滑鼠按下(右)"); break;
#else
                instance.RecordCmd(L"MouseDown(Right)"); break;
#endif // _LANG_ZH_TW_
            case WM_RBUTTONUP:   
#ifdef _LANG_ZH_TW_
                instance.RecordCmd(L"滑鼠放開(右)"); break;
#else
                instance.RecordCmd(L"MouseUp(Right)"); break;
#endif // _LANG_ZH_TW_
            case WM_MBUTTONDOWN: 
#ifdef _LANG_ZH_TW_
                instance.RecordCmd(L"滑鼠按下(中)"); break;
#else
                instance.RecordCmd(L"MouseDown(Middle)"); break;
#endif // _LANG_ZH_TW_
            case WM_MBUTTONUP:   
#ifdef _LANG_ZH_TW_
                instance.RecordCmd(L"滑鼠放開(中)"); break;
#else
                instance.RecordCmd(L"MouseUp(Middle)"); break;
#endif // _LANG_ZH_TW_
            case WM_MOUSEWHEEL: {
                short delta = GET_WHEEL_DELTA_WPARAM(mouse->mouseData);
#ifdef _LANG_ZH_TW_
                instance.RecordCmd(delta > 0 ? L"滾輪上()" : L"滾輪下()");
#else
                instance.RecordCmd(delta > 0 ? L"WheelUp()" : L"WheelDown()");
#endif // _LANG_ZH_TW_
                break;
            }
            case WM_MOUSEMOVE:
#ifdef _LANG_ZH_TW_
                instance.RecordCmd(std::format(L"絕對移動({}, {})", mouse->pt.x, mouse->pt.y));
#else
                instance.RecordCmd(std::format(L"AbsoluteMove({}, {})", mouse->pt.x, mouse->pt.y));
#endif // _LANG_ZH_TW_
                break;
            }
        }
    }
    return ::CallNextHookEx(instance.m_mouseHook, nCode, wParam, lParam);
}

void RecordHelper::StartRecording() {
    m_lastRecordTime = std::chrono::steady_clock::now();
    m_isRecording = true;
    m_mouseHook = SetWindowsHookExW(WH_MOUSE_LL, RecordHelper::LowLevelMouseProc, GetModuleHandle(nullptr), 0);
    m_keyboardHook = SetWindowsHookExW(WH_KEYBOARD_LL, RecordHelper::LowLevelKeyboardProc, GetModuleHandle(nullptr), 0);
}

void RecordHelper::StopRecording() {
    m_isRecording = false;
    if (m_mouseHook) UnhookWindowsHookEx(m_mouseHook);
    if (m_keyboardHook) UnhookWindowsHookEx(m_keyboardHook);
    m_mouseHook = nullptr;
    m_keyboardHook = nullptr;
}

bool RecordHelper::GetCommands(std::vector<ParsedCommand>& outParsed, std::wstring& outErrorMsg, int& outErrorLine) {
    std::vector<std::wstring> copy;
    {
        std::lock_guard lock(m_scriptMutex);
        copy = m_scriptBuffer;
    }
    HotkeyData startKey;
    HotkeyData endKey;
    ScriptRunMode runMode;
    if (!ValidateAndParseScript(copy, startKey, endKey, runMode, outParsed, outErrorMsg, outErrorLine)) {
        return false;
    }
    return true;
}

void RecordHelper::StartPlayback(const std::vector<ParsedCommand>& commands, std::function<void()> preExecute, std::function<void(bool hasError)> postExecute) {
    if (m_scriptData.isRunning) return;
    m_scriptData.commands = commands;
    m_scriptData.mode = ScriptRunMode::Single;
    m_scriptData.isEnabled = true;
    m_scriptData.preExecute = preExecute;
    m_scriptData.postExecute = postExecute;
    StartScriptExecution(m_scriptData);
}

void RecordHelper::StopPlayback() {
    StopScriptExecution(m_scriptData, true);
}

void RecordHelper::GetScriptBuffer(std::vector<std::wstring>& outBuffer) {
    std::lock_guard lock(m_scriptMutex);
    outBuffer = m_scriptBuffer;
}

void RecordHelper::SetScriptBuffer(const std::vector<std::wstring>& inBuffer) {
    std::lock_guard lock(m_scriptMutex);
    m_scriptBuffer = inBuffer;
}

bool RecordHelper::IsPlaying() {
    return m_scriptData.isRunning;
}

bool RecordHelper::IsRecording() {
    return m_isRecording;
}

#include "pch.h"
#include "RecordHelper.h"

RecordHelper& RecordHelper::GetInstance() {
    static RecordHelper instance;
    return instance;
}

void RecordHelper::RecordCmd(const std::u8string& cmd) {
    auto now = std::chrono::steady_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - m_lastRecordTime).count();
    m_lastRecordTime = now;

    std::lock_guard lock(m_scriptMutex);
    if (elapsed > 10) {
        std::u8string delayName = u8"Delay";
        if (I18n::GetLanguage() == Language::TraditionalChinese || I18n::GetLanguage() == Language::Cantonese) {
            delayName = u8"延遲";
        } else if (I18n::GetLanguage() == Language::SimplifiedChinese) {
            delayName = u8"延迟";
        }
        m_scriptBuffer.push_back(delayName + ToU8StringUnchecked(std::format("({})", elapsed)));
    }
    m_scriptBuffer.push_back(cmd);
}

LRESULT CALLBACK RecordHelper::LowLevelKeyboardProc(int nCode, WPARAM wParam, LPARAM lParam) {
    auto& instance = RecordHelper::GetInstance();
    if (nCode >= 0 && instance.m_isRecording) {
        auto* kbd = reinterpret_cast<KBDLLHOOKSTRUCT*>(lParam);
        if (!(kbd->flags & LLKHF_INJECTED)) {
            auto keyName = GetNameByKey(kbd->vkCode);
            std::u8string fnName;
            if (wParam == WM_KEYDOWN || wParam == WM_SYSKEYDOWN) {
                if (I18n::GetLanguage() == Language::TraditionalChinese || I18n::GetLanguage() == Language::Cantonese) {
                    fnName = u8"鍵盤按下";
                } else if (I18n::GetLanguage() == Language::SimplifiedChinese) {
                    fnName = u8"键盘按下";
                } else {
                    fnName = u8"KeyboardDown";
                }
            } else if (wParam == WM_KEYUP || wParam == WM_SYSKEYUP) {
                if (I18n::GetLanguage() == Language::TraditionalChinese || I18n::GetLanguage() == Language::Cantonese) {
                    fnName = u8"鍵盤放開";
                } else if (I18n::GetLanguage() == Language::SimplifiedChinese) {
                    fnName = u8"键盘放开";
                } else {
                    fnName = u8"KeyboardUp";
                }
            }
            if (!fnName.empty()) {
                if (keyName.has_value()) {
                    instance.RecordCmd(ToU8StringUnchecked(std::format("{}({})", ToStringUnchecked(fnName), ToStringUnchecked(keyName.value()))));
                } else {
                    instance.RecordCmd(ToU8StringUnchecked(std::format("{}(0x{:X})", ToStringUnchecked(fnName), kbd->vkCode)));
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
            bool isTradOrCant = (I18n::GetLanguage() == Language::TraditionalChinese || I18n::GetLanguage() == Language::Cantonese);
            bool isSimp = (I18n::GetLanguage() == Language::SimplifiedChinese);
            switch (wParam) {
            case WM_LBUTTONDOWN: 
                if (isTradOrCant) instance.RecordCmd(u8"滑鼠按下(左)");
                else if (isSimp) instance.RecordCmd(u8"鼠标按下(左)");
                else instance.RecordCmd(u8"MouseDown(Left)");
                break;
            case WM_LBUTTONUP:   
                if (isTradOrCant) instance.RecordCmd(u8"滑鼠放開(左)");
                else if (isSimp) instance.RecordCmd(u8"鼠标放开(左)");
                else instance.RecordCmd(u8"MouseUp(Left)");
                break;
            case WM_RBUTTONDOWN: 
                if (isTradOrCant) instance.RecordCmd(u8"滑鼠按下(右)");
                else if (isSimp) instance.RecordCmd(u8"鼠标按下(右)");
                else instance.RecordCmd(u8"MouseDown(Right)");
                break;
            case WM_RBUTTONUP:   
                if (isTradOrCant) instance.RecordCmd(u8"滑鼠放開(右)");
                else if (isSimp) instance.RecordCmd(u8"鼠标放开(右)");
                else instance.RecordCmd(u8"MouseUp(Right)");
                break;
            case WM_MBUTTONDOWN: 
                if (isTradOrCant) instance.RecordCmd(u8"滑鼠按下(中)");
                else if (isSimp) instance.RecordCmd(u8"鼠标按下(中)");
                else instance.RecordCmd(u8"MouseDown(Middle)");
                break;
            case WM_MBUTTONUP:   
                if (isTradOrCant) instance.RecordCmd(u8"滑鼠放開(中)");
                else if (isSimp) instance.RecordCmd(u8"鼠标放开(中)");
                else instance.RecordCmd(u8"MouseUp(Middle)");
                break;
            case WM_MOUSEWHEEL: {
                short delta = GET_WHEEL_DELTA_WPARAM(mouse->mouseData);
                if (isTradOrCant) instance.RecordCmd(delta > 0 ? u8"滾輪上()" : u8"滾輪下()");
                else if (isSimp) instance.RecordCmd(delta > 0 ? u8"滚轮上()" : u8"滚轮下()");
                else instance.RecordCmd(delta > 0 ? u8"WheelUp()" : u8"WheelDown()");
                break;
            }
            case WM_MOUSEMOVE:
                if (isTradOrCant) instance.RecordCmd(u8"絕對移動" + ToU8StringUnchecked(std::format("({}, {})", mouse->pt.x, mouse->pt.y)));
                else if (isSimp) instance.RecordCmd(u8"绝对移动" + ToU8StringUnchecked(std::format("({}, {})", mouse->pt.x, mouse->pt.y)));
                else instance.RecordCmd(u8"AbsoluteMove" + ToU8StringUnchecked(std::format("({}, {})", mouse->pt.x, mouse->pt.y)));
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

bool RecordHelper::GetCommands(std::vector<ParsedCommand>& outParsed, std::u8string& outErrorMsg, int& outErrorLine) {
    std::vector<std::u8string> copy;
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
    m_scriptData.mode = ScriptRunMode::FullSingle;
    m_scriptData.isEnabled = true;
    m_scriptData.preExecute = preExecute;
    m_scriptData.postExecute = postExecute;
    StartScriptExecution(m_scriptData);
}

void RecordHelper::StopPlayback() {
    StopScriptExecution(m_scriptData, true, true);
}

void RecordHelper::GetScriptBuffer(std::vector<std::u8string>& outBuffer) {
    std::lock_guard lock(m_scriptMutex);
    outBuffer = m_scriptBuffer;
}

void RecordHelper::SetScriptBuffer(const std::vector<std::u8string>& inBuffer) {
    std::lock_guard lock(m_scriptMutex);
    m_scriptBuffer = inBuffer;
}

bool RecordHelper::IsPlaying() {
    return m_scriptData.isRunning;
}

bool RecordHelper::IsRecording() {
    return m_isRecording;
}

#pragma once

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
#include "Resource.h"
#include "Utils.h"

class RecordHelper {
public:
    static RecordHelper& GetInstance();
    void StartRecording();
    void StopRecording();
    bool GetCommands(std::vector<ParsedCommand>& outParsed, std::wstring& outErrorMsg, int& outErrorLine);
    void StartPlayback(const std::vector<ParsedCommand>& commands, std::function<void()> preExecute, std::function<void(bool hasError)> postExecute);
    void StopPlayback();
    void GetScriptBuffer(std::vector<std::wstring>& outBuffer);
    void SetScriptBuffer(const std::vector<std::wstring>& inBuffer);
    bool IsPlaying();
    bool IsRecording();
private:
    HHOOK m_mouseHook = nullptr;
    HHOOK m_keyboardHook = nullptr;

    std::atomic<bool> m_isRecording;
    std::mutex m_scriptMutex;
    ScriptData m_scriptData;
    std::vector<std::wstring> m_scriptBuffer;
    std::chrono::steady_clock::time_point m_lastRecordTime;
    void RecordCmd(const std::wstring& cmd);
    static LRESULT LowLevelKeyboardProc(int nCode, WPARAM wParam, LPARAM lParam);
    static LRESULT LowLevelMouseProc(int nCode, WPARAM wParam, LPARAM lParam);
};
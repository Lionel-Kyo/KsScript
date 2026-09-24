#pragma once
#ifndef _RECORD_HELPER_H_
#define _RECORD_HELPER_H_

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
#include "res/Resource.h"
#include "Utils.h"
#include "XorString.h"

class RecordHelper {
public:
    static RecordHelper& GetInstance();
    void StartRecording();
    void StopRecording();
    bool GetCommands(std::vector<ParsedCommand>& outParsed, std::u8string& outErrorMsg, int& outErrorLine);
    void StartPlayback(const std::vector<ParsedCommand>& commands, std::function<void()> preExecute, std::function<void(bool hasError)> postExecute);
    void StopPlayback();
    void GetScriptBuffer(std::vector<std::u8string>& outBuffer);
    void SetScriptBuffer(const std::vector<std::u8string>& inBuffer);
    bool IsPlaying();
    bool IsRecording();
private:
    HHOOK m_mouseHook = nullptr;
    HHOOK m_keyboardHook = nullptr;

    std::atomic<bool> m_isRecording;
    std::mutex m_scriptMutex;
    ScriptData m_scriptData;
    std::vector<std::u8string> m_scriptBuffer;
    std::chrono::steady_clock::time_point m_lastRecordTime;
    void RecordCmd(const std::u8string& cmd);
    static LRESULT LowLevelKeyboardProc(int nCode, WPARAM wParam, LPARAM lParam);
    static LRESULT LowLevelMouseProc(int nCode, WPARAM wParam, LPARAM lParam);
};

#endif // !_RECORD_HELPER_H_
#pragma once
#include <windows.h>
#include <dwmapi.h>
#include <any>
#include <fstream>
#include <sstream>
#include <string>
#include <string_view>
#include <vector>
#include <thread>
#include <chrono>
#include <condition_variable>
#include <mutex>
#include <atomic>
#include <ranges>
#include <algorithm>
#include <map>
#include <optional>
#include <cwctype>
#include <functional>
#include "Theme/ThemeColors.h"

enum class CmdType {
    None,
    SetStartKey,
    SetEndKey,
    SetScriptRunMode,
    Delay,
    MouseDown,
    MouseUp,
    MouseAbsoluteMove,
    MouseRelativeMove,
    ShowCoordinate,
    KeyboardDown,
    KeyboardUp,
    WheelUp,
    WheelDown,
    OutputText
};

enum class MouseButtonType {
    Left,
    Right,
    Middle,
    X1,
    X2
};
std::optional<MouseButtonType> MouseButtonTypeFromString(const std::wstring& str);

inline std::map<std::wstring, int32_t> NAME_MATCH_KEYS = {
   //{ L"KeyCode", 0xFFFF },
   //{ L"Modifiers", -65536 },
   { L"None", 0 },
   { L"LButton", 1 },
   { L"RButton", 2 },
   { L"Cancel", 3 },
   { L"MButton", 4 },
   { L"XButton1", 5 },
   { L"XButton2", 6 },
   { L"Back", 8 },
   { L"Tab", 9 },
   { L"LineFeed", 0xA },
   { L"Clear", 0xC },
   { L"Return", 0xD },
   { L"Enter", 0xD },
   { L"ShiftKey", 0x10 },
   { L"ControlKey", 0x11 },
   { L"Menu", 0x12 },
   { L"Pause", 0x13 },
   { L"Capital", 0x14 },
   { L"CapsLock", 0x14 },
   { L"KanaMode", 0x15 },
   { L"HanguelMode", 0x15 },
   { L"HangulMode", 0x15 },
   { L"JunjaMode", 0x17 },
   { L"FinalMode", 0x18 },
   { L"HanjaMode", 0x19 },
   { L"KanjiMode", 0x19 },
   { L"Escape", 0x1B },
   { L"IMEConvert", 0x1C },
   { L"IMENonconvert", 0x1D },
   { L"IMEAccept", 0x1E },
   { L"IMEAceept", 0x1E },
   { L"IMEModeChange", 0x1F },
   { L"Space", 0x20 },
   { L"Prior", 0x21 },
   { L"PageUp", 0x21 },
   { L"Next", 0x22 },
   { L"PageDown", 0x22 },
   { L"End", 0x23 },
   { L"Home", 0x24 },
   { L"Left", 0x25 },
   { L"Up", 0x26 },
   { L"Right", 0x27 },
   { L"Down", 0x28 },
   { L"Select", 0x29 },
   { L"Print", 0x2A },
   { L"Execute", 0x2B },
   { L"Snapshot", 0x2C },
   { L"PrintScreen", 0x2C },
   { L"Insert", 0x2D },
   { L"Delete", 0x2E },
   { L"Help", 0x2F },
   { L"D0", 0x30 },
   { L"D1", 0x31 },
   { L"D2", 0x32 },
   { L"D3", 0x33 },
   { L"D4", 0x34 },
   { L"D5", 0x35 },
   { L"D6", 0x36 },
   { L"D7", 0x37 },
   { L"D8", 0x38 },
   { L"D9", 0x39 },
   { L"A", 0x41 },
   { L"B", 0x42 },
   { L"C", 0x43 },
   { L"D", 0x44 },
   { L"E", 0x45 },
   { L"F", 0x46 },
   { L"G", 0x47 },
   { L"H", 0x48 },
   { L"I", 0x49 },
   { L"J", 0x4A },
   { L"K", 0x4B },
   { L"L", 0x4C },
   { L"M", 0x4D },
   { L"N", 0x4E },
   { L"O", 0x4F },
   { L"P", 0x50 },
   { L"Q", 0x51 },
   { L"R", 0x52 },
   { L"S", 0x53 },
   { L"T", 0x54 },
   { L"U", 0x55 },
   { L"V", 0x56 },
   { L"W", 0x57 },
   { L"X", 0x58 },
   { L"Y", 0x59 },
   { L"Z", 0x5A },
   { L"LWin", 0x5B },
   { L"RWin", 0x5C },
   { L"Apps", 0x5D },
   { L"Sleep", 0x5F },
   { L"NumPad0", 0x60 },
   { L"NumPad1", 0x61 },
   { L"NumPad2", 0x62 },
   { L"NumPad3", 0x63 },
   { L"NumPad4", 0x64 },
   { L"NumPad5", 0x65 },
   { L"NumPad6", 0x66 },
   { L"NumPad7", 0x67 },
   { L"NumPad8", 0x68 },
   { L"NumPad9", 0x69 },
   { L"Multiply", 0x6A },
   { L"Add", 0x6B },
   { L"Separator", 0x6C },
   { L"Subtract", 0x6D },
   { L"Decimal", 0x6E },
   { L"Divide", 0x6F },
   { L"F1", 0x70 },
   { L"F2", 0x71 },
   { L"F3", 0x72 },
   { L"F4", 0x73 },
   { L"F5", 0x74 },
   { L"F6", 0x75 },
   { L"F7", 0x76 },
   { L"F8", 0x77 },
   { L"F9", 0x78 },
   { L"F10", 0x79 },
   { L"F11", 0x7A },
   { L"F12", 0x7B },
   { L"F13", 0x7C },
   { L"F14", 0x7D },
   { L"F15", 0x7E },
   { L"F16", 0x7F },
   { L"F17", 0x80 },
   { L"F18", 0x81 },
   { L"F19", 0x82 },
   { L"F20", 0x83 },
   { L"F21", 0x84 },
   { L"F22", 0x85 },
   { L"F23", 0x86 },
   { L"F24", 0x87 },
   { L"NumLock", 0x90 },
   { L"Scroll", 0x91 },
   { L"LShiftKey", 0xA0 },
   { L"RShiftKey", 0xA1 },
   { L"LControlKey", 0xA2 },
   { L"RControlKey", 0xA3 },
   { L"LMenu", 0xA4 },
   { L"RMenu", 0xA5 },
   { L"BrowserBack", 0xA6 },
   { L"BrowserForward", 0xA7 },
   { L"BrowserRefresh", 0xA8 },
   { L"BrowserStop", 0xA9 },
   { L"BrowserSearch", 0xAA },
   { L"BrowserFavorites", 0xAB },
   { L"BrowserHome", 0xAC },
   { L"VolumeMute", 0xAD },
   { L"VolumeDown", 0xAE },
   { L"VolumeUp", 0xAF },
   { L"MediaNextTrack", 0xB0 },
   { L"MediaPreviousTrack", 0xB1 },
   { L"MediaStop", 0xB2 },
   { L"MediaPlayPause", 0xB3 },
   { L"LaunchMail", 0xB4 },
   { L"SelectMedia", 0xB5 },
   { L"LaunchApplication1", 0xB6 },
   { L"LaunchApplication2", 0xB7 },
   { L"OemSemicolon", 0xBA },
   { L"Oem1", 0xBA },
   { L"Oemplus", 0xBB },
   { L"Oemcomma", 0xBC },
   { L"OemMinus", 0xBD },
   { L"OemPeriod", 0xBE },
   { L"OemQuestion", 0xBF },
   { L"Oem2", 0xBF },
   { L"Oemtilde", 0xC0 },
   { L"Oem3", 0xC0 },
   { L"OemOpenBrackets", 0xDB },
   { L"Oem4", 0xDB },
   { L"OemPipe", 0xDC },
   { L"Oem5", 0xDC },
   { L"OemCloseBrackets", 0xDD },
   { L"Oem6", 0xDD },
   { L"OemQuotes", 0xDE },
   { L"Oem7", 0xDE },
   { L"Oem8", 0xDF },
   { L"OemBackslash", 0xE2 },
   { L"Oem102", 0xE2 },
   { L"ProcessKey", 0xE5 },
   { L"Packet", 0xE7 },
   { L"Attn", 0xF6 },
   { L"Crsel", 0xF7 },
   { L"Exsel", 0xF8 },
   { L"EraseEof", 0xF9 },
   { L"Play", 0xFA },
   { L"Zoom", 0xFB },
   { L"NoName", 0xFC },
   { L"Pa1", 0xFD },
   { L"OemClear", 0xFE },
   //{ L"Shift", 0x10000 },
   //{ L"Control", 0x20000 },
   //{ L"Alt", 0x40000 }
};
std::optional<int32_t> GetKeyByName(std::wstring_view name);
std::optional<std::wstring> GetNameByKey(int32_t vk);

namespace InputSim {
    bool Delay(const std::atomic<bool>& isForceStopRequested, int64_t ms);
    void MouseDown(MouseButtonType type);
    void MouseUp(MouseButtonType type);
    void MouseAbsoluteMove(int32_t x, int32_t y);
    void MouseRelativeMove(int32_t dx, int32_t dy);
    bool IsExtendedKey(uint16_t vkCode);
    void KeyboardDown(uint16_t vkCode);
    void KeyboardUp(uint16_t vkCode);
    void Wheel(bool up);
    void ShowCoordinate();
    void OutputText(const std::atomic<bool>& isForceStopRequested, const std::wstring& text, int64_t ms);
}

enum class ScriptRunMode {
    // Complete full command batch before stopping
    FullSingle,
    FullContinuous,
    FullSwitch,

    // Stop immediately on current command
    Single,
    Continuous,
    Switch
};

std::optional<ScriptRunMode> ScriptRunModeFromString(const std::wstring& str);
std::wstring ScriptRunModeToString(ScriptRunMode mode);

enum class KeyModifierRequirement {
    Any,
    MustDown,
    MustUp
};

struct HotkeyData {
    int32_t key{};

    KeyModifierRequirement ctrl{ KeyModifierRequirement::Any };
    KeyModifierRequirement shift{ KeyModifierRequirement::Any };
    KeyModifierRequirement alt{ KeyModifierRequirement::Any };
};

std::wstring HotkeyDataToString(const HotkeyData& data);

struct OutputTextData {
    std::wstring text;
    int64_t delayMs{ 0 };
};

struct KeyboardEventData {
    int32_t vkCode;
    bool isDown;
    bool isLCtrlDown;
    bool isRCtrlDown;
    bool isLShiftDown;
    bool isRShiftDown;
    bool isLAltDown;
    bool isRAltDown;
};

bool CheckIsHotkeyDown(const HotkeyData& hotkeyData, const KeyboardEventData& eventData);

struct ParsedCommand {
    CmdType type = CmdType::None;
    std::any args;
};

struct ScriptData {
    std::vector<ParsedCommand> commands;
    HotkeyData startKey;
    HotkeyData endKey;
    ScriptRunMode mode{ ScriptRunMode::FullSingle };
    std::atomic<bool> isEnabled{ false };
    std::atomic<bool> isRunning{ false };
    std::atomic<bool> isForceStopRequested{ false };
    std::jthread executeThread;
    std::function<void()> preExecute;
    std::function<void(bool hasError)> postExecute;

    ScriptData() = default;

    ScriptData(const ScriptData&) = delete;
    ScriptData& operator=(const ScriptData&) = delete;

    ScriptData(ScriptData&& other) noexcept
        : commands(std::move(other.commands))
        , startKey(other.startKey)
        , endKey(other.endKey)
        , mode(other.mode)
        , isEnabled(other.isEnabled.load())
        , isRunning(other.isRunning.load())
        , isForceStopRequested(other.isForceStopRequested.load())
        , executeThread(std::move(other.executeThread))
        , preExecute(std::move(other.preExecute))
        , postExecute(std::move(other.postExecute)) {}

    ScriptData& operator=(ScriptData&& other) noexcept {
        if (this != &other) {
            commands = std::move(other.commands);
            startKey = other.startKey;
            endKey = other.endKey;
            mode = other.mode;
            isEnabled.store(other.isEnabled.load());
            isRunning.store(other.isRunning.load());
            isForceStopRequested.store(other.isForceStopRequested.load());
            executeThread = std::move(other.executeThread);
            preExecute = std::move(other.preExecute);
            postExecute = std::move(other.postExecute);
        }

        return *this;
    }
};

struct ScriptEntry {
    std::wstring fileName;
    std::wstring filePath;
    std::vector<std::wstring> lines;
    ScriptData data;
};

std::wstring Utf8ToUtf16(const std::string& utf8Str);
std::string Utf16ToUtf8(const std::wstring& utf16Str);

bool ParseScriptLine(const std::wstring& line, ParsedCommand& outCmd, std::wstring& outError);
bool ValidateAndParseScript(
    const std::vector<std::wstring>& liness, 
    HotkeyData& outStartKey,
    HotkeyData& outEndKey,
    ScriptRunMode& outRunMode, 
    std::vector<ParsedCommand>& outParsed, 
    std::wstring& outErrorMsg, int& outErrorLine
);

bool ReadUtf8FileLines(const std::wstring& filePath, std::vector<std::wstring>& outLines);

bool StartScriptExecution(ScriptData& data);
bool StopScriptExecution(ScriptData& data, bool forceStop, bool waitScriptEnd);

bool CListCtrlPreTranslateMessageClipboard(MSG* pMsg, const CDialog* dlg, const CListCtrl& listCtrl);
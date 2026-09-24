#pragma once
#ifndef _UTILS_H_
#define _UTILS_H_

#include <windows.h>
#include <dwmapi.h>
#include <any>
#include <fstream>
#include <sstream>
#include <filesystem>
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
#include "XorString.h"

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
    KeyboardDown,
    KeyboardUp,
    WheelUp,
    WheelDown,
    HoldTitleBar,
    ReleaseTitleBar,
    OutputText,
    ShowCoordinate,
};

enum class MouseButtonType {
    Left,
    Right,
    Middle,
    X1,
    X2
};
std::optional<MouseButtonType> MouseButtonTypeFromString(const std::u8string& str);

inline std::map<std::u8string, int32_t> NAME_MATCH_KEYS = {
    // General & Mouse
    { u8"None", 0 },
    { u8"LButton", 1 },
    { u8"RButton", 2 },
    { u8"Cancel", 3 },
    { u8"MButton", 4 },
    { u8"XButton1", 5 },
    { u8"XButton2", 6 },
    { u8"Back", 8 },
    { u8"Tab", 9 },
    { u8"LineFeed", 0xA },
    { u8"Clear", 0xC },
    { u8"Return", 0xD },
    { u8"Enter", 0xD },

    // Modifiers & System
    { u8"Shift", 0x10 },
    { u8"ShiftKey", 0x10 },
    { u8"Ctrl", 0x11 },
    { u8"ControlKey", 0x11 },
    { u8"Menu", 0x12 },
    { u8"Pause", 0x13 },
    { u8"CapsLock", 0x14 },
    { u8"Capital", 0x14 },
    { u8"KanaMode", 0x15 },
    { u8"HanguelMode", 0x15 },
    { u8"HangulMode", 0x15 },
    { u8"JunjaMode", 0x17 },
    { u8"FinalMode", 0x18 },
    { u8"HanjaMode", 0x19 },
    { u8"KanjiMode", 0x19 },
    { u8"Escape", 0x1B },

    // IME
    { u8"IMEConvert", 0x1C },
    { u8"IMENonconvert", 0x1D },
    { u8"IMEAccept", 0x1E },
    { u8"IMEModeChange", 0x1F },

    // Navigation & Editing
    { u8"Space", 0x20 },
    { u8"PageUp", 0x21 },
    { u8"Prior", 0x21 },
    { u8"PageDown", 0x22 },
    { u8"Next", 0x22 },
    { u8"End", 0x23 },
    { u8"Home", 0x24 },
    { u8"Left", 0x25 },
    { u8"Up", 0x26 },
    { u8"Right", 0x27 },
    { u8"Down", 0x28 },
    { u8"Select", 0x29 },
    { u8"Print", 0x2A },
    { u8"Execute", 0x2B },
    { u8"PrintScreen", 0x2C },
    { u8"Snapshot", 0x2C },
    { u8"Insert", 0x2D },
    { u8"Delete", 0x2E },
    { u8"Help", 0x2F },

    // Numbers
    { u8"D0", 0x30 }, { u8"D1", 0x31 }, { u8"D2", 0x32 }, { u8"D3", 0x33 }, { u8"D4", 0x34 },
    { u8"D5", 0x35 }, { u8"D6", 0x36 }, { u8"D7", 0x37 }, { u8"D8", 0x38 }, { u8"D9", 0x39 },

    // Alphabet
    { u8"A", 0x41 }, { u8"B", 0x42 }, { u8"C", 0x43 }, { u8"D", 0x44 }, { u8"E", 0x45 },
    { u8"F", 0x46 }, { u8"G", 0x47 }, { u8"H", 0x48 }, { u8"I", 0x49 }, { u8"J", 0x4A },
    { u8"K", 0x4B }, { u8"L", 0x4C }, { u8"M", 0x4D }, { u8"N", 0x4E }, { u8"O", 0x4F },
    { u8"P", 0x50 }, { u8"Q", 0x51 }, { u8"R", 0x52 }, { u8"S", 0x53 }, { u8"T", 0x54 },
    { u8"U", 0x55 }, { u8"V", 0x56 }, { u8"W", 0x57 }, { u8"X", 0x58 }, { u8"Y", 0x59 },
    { u8"Z", 0x5A },

    // Windows Keys & Apps
    { u8"LWin", 0x5B },
    { u8"RWin", 0x5C },
    { u8"Apps", 0x5D },
    { u8"Sleep", 0x5F },

    // Numpad
    { u8"NumPad0", 0x60 }, { u8"NumPad1", 0x61 }, { u8"NumPad2", 0x62 }, { u8"NumPad3", 0x63 },
    { u8"NumPad4", 0x64 }, { u8"NumPad5", 0x65 }, { u8"NumPad6", 0x66 }, { u8"NumPad7", 0x67 },
    { u8"NumPad8", 0x68 }, { u8"NumPad9", 0x69 },
    { u8"Multiply", 0x6A },
    { u8"Add", 0x6B },
    { u8"Separator", 0x6C },
    { u8"Subtract", 0x6D },
    { u8"Decimal", 0x6E },
    { u8"Divide", 0x6F },

    // Function Keys
    { u8"F1", 0x70 }, { u8"F2", 0x71 }, { u8"F3", 0x72 }, { u8"F4", 0x73 },
    { u8"F5", 0x74 }, { u8"F6", 0x75 }, { u8"F7", 0x76 }, { u8"F8", 0x77 },
    { u8"F9", 0x78 }, { u8"F10", 0x79 }, { u8"F11", 0x7A }, { u8"F12", 0x7B },
    { u8"F13", 0x7C }, { u8"F14", 0x7D }, { u8"F15", 0x7E }, { u8"F16", 0x7F },
    { u8"F17", 0x80 }, { u8"F18", 0x81 }, { u8"F19", 0x82 }, { u8"F20", 0x83 },
    { u8"F21", 0x84 }, { u8"F22", 0x85 }, { u8"F23", 0x86 }, { u8"F24", 0x87 },

    // Locks & Specific Modifiers
    { u8"NumLock", 0x90 },
    { u8"Scroll", 0x91 },
    { u8"LShift", 0xA0 },
    { u8"LShiftKey", 0xA0 },
    { u8"RShift", 0xA1 },
    { u8"RShiftKey", 0xA1 },
    { u8"LCtrl", 0xA2 },
    { u8"LControlKey", 0xA2 },
    { u8"RCtrl", 0xA3 },
    { u8"RControlKey", 0xA3 },
    { u8"LAlt", 0xA4 },
    { u8"LMenu", 0xA4 },
    { u8"RAlt", 0xA5 },
    { u8"RMenu", 0xA5 },

    // Browser & Media
    { u8"BrowserBack", 0xA6 },
    { u8"BrowserForward", 0xA7 },
    { u8"BrowserRefresh", 0xA8 },
    { u8"BrowserStop", 0xA9 },
    { u8"BrowserSearch", 0xAA },
    { u8"BrowserFavorites", 0xAB },
    { u8"BrowserHome", 0xAC },
    { u8"VolumeMute", 0xAD },
    { u8"VolumeDown", 0xAE },
    { u8"VolumeUp", 0xAF },
    { u8"MediaNextTrack", 0xB0 },
    { u8"MediaPreviousTrack", 0xB1 },
    { u8"MediaStop", 0xB2 },
    { u8"MediaPlayPause", 0xB3 },
    { u8"LaunchMail", 0xB4 },
    { u8"SelectMedia", 0xB5 },
    { u8"LaunchApplication1", 0xB6 },
    { u8"LaunchApplication2", 0xB7 },

    // OEM Keys
    { u8"Semicolon", 0xBA }, { u8"OemSemicolon", 0xBA },
    { u8"Oem1", 0xBA },
    { u8"Plus", 0xBB }, { u8"Oemplus", 0xBB },
    { u8"Comma", 0xBC }, { u8"Oemcomma", 0xBC },
    { u8"Minus", 0xBD }, { u8"OemMinus", 0xBD },
    { u8"Period", 0xBE }, { u8"OemPeriod", 0xBE },
    { u8"Slash", 0xBF }, { u8"OemQuestion", 0xBF },
    { u8"Oem2", 0xBF },
    { u8"Tilde", 0xC0 }, { u8"Oemtilde", 0xC0 },
    { u8"Oem3", 0xC0 },
    { u8"LeftBracket", 0xDB }, { u8"OemOpenBrackets", 0xDB },
    { u8"Oem4", 0xDB },
    { u8"Pipe", 0xDC }, { u8"OemPipe", 0xDC },
    { u8"Oem5", 0xDC },
    { u8"RightBracket", 0xDD }, { u8"OemCloseBrackets", 0xDD },
    { u8"Oem6", 0xDD },
    { u8"Quote", 0xDE }, { u8"OemQuotes", 0xDE },
    { u8"Oem7", 0xDE },
    { u8"Oem8", 0xDF },
    { u8"Backslash", 0xE2 }, { u8"OemBackslash", 0xE2 },
    { u8"Oem102", 0xE2 },

    // Miscellaneous
    { u8"ProcessKey", 0xE5 },
    { u8"Packet", 0xE7 },
    { u8"Attn", 0xF6 },
    { u8"Crsel", 0xF7 },
    { u8"Exsel", 0xF8 },
    { u8"EraseEof", 0xF9 },
    { u8"Play", 0xFA },
    { u8"Zoom", 0xFB },
    { u8"NoName", 0xFC },
    { u8"Pa1", 0xFD },
    { u8"OemClear", 0xFE }
};

std::optional<int32_t> GetKeyByName(std::u8string_view name);
std::optional<std::u8string> GetNameByKey(int32_t vk);

std::u8string wstringToU8String(const std::wstring& wstr);
std::wstring u8stringToWString(const std::u8string& u8str);
std::u8string ToU8StringUnchecked(const std::string& str);
std::string ToStringUnchecked(const std::u8string& str);

std::u8string_view StringTrim(std::u8string_view input);
std::wstring StringLower(const std::wstring& input);
std::u8string StringLower(std::u8string_view input);

bool StringEqualIgnoreCase(std::u8string_view a, std::u8string_view b);

std::istream& u8getline(std::istream& input, std::u8string& u8str, char8_t delim = u8'\n');



namespace InputSim {
    struct WindowSearchData {
        std::wstring targetName;
        HWND foundHwnd = nullptr;
    };
    BOOL CALLBACK EnumWindowsCallback(HWND hWnd, LPARAM lParam);
    HWND FindVisibleWindows(const std::wstring& name);

    bool Delay(const std::atomic<bool>& isForceStopRequested, int64_t ms);
    void MouseDown(MouseButtonType type);
    void MouseUp(MouseButtonType type);
    void MouseAbsoluteMove(int32_t x, int32_t y);
    void MouseRelativeMove(int32_t dx, int32_t dy);
    bool IsExtendedKey(uint16_t vkCode);
    void KeyboardDown(uint16_t vkCode);
    void KeyboardUp(uint16_t vkCode);
    void Wheel(bool up);
    void HoldTitleBar(const std::wstring& windowName);
    void ReleaseTitleBar(const std::wstring& windowName);
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

std::optional<ScriptRunMode> ScriptRunModeFromString(const std::u8string& str);
std::u8string ScriptRunModeToString(ScriptRunMode mode);

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

std::u8string HotkeyDataToString(const HotkeyData& data);

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
    std::u8string fileName;
    std::u8string filePath;
    std::vector<std::u8string> lines;
    ScriptData data;
};

bool ParseScriptLine(const std::u8string& line, ParsedCommand& outCmd, std::u8string& outError);
bool ValidateAndParseScript(
    const std::vector<std::u8string>& liness, 
    HotkeyData& outStartKey,
    HotkeyData& outEndKey,
    ScriptRunMode& outRunMode, 
    std::vector<ParsedCommand>& outParsed, 
    std::u8string& outErrorMsg, int& outErrorLine
);

bool ReadUtf8FileLines(const std::u8string& filePath, std::vector<std::u8string>& outLines);

bool StartScriptExecution(ScriptData& data);
bool StopScriptExecution(ScriptData& data, bool forceStop, bool waitScriptEnd);

#endif // !_UTILS_H_
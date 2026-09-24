#include "pch.h"
#include "Utils.h"
#include <chrono>
#include <sstream>
#include <format>

std::optional<int32_t> GetKeyByName(std::u8string_view name) {
    for (const auto& [key, value] : NAME_MATCH_KEYS) {
        if (StringEqualIgnoreCase(key, name)) {
            return value;
        }
    }
    return std::nullopt;
}

std::optional<std::u8string> GetNameByKey(int32_t vk) {
    for (const auto& [key, value] : NAME_MATCH_KEYS) {
        if (value == vk) {
            return key;
        }
    }
    return std::nullopt;
}

std::u8string wstringToU8String(const std::wstring& wstr) {
    if (wstr.empty()) {
        return std::u8string();
    }

    int sizeNeeded = WideCharToMultiByte(CP_UTF8, 0, wstr.data(), static_cast<int>(wstr.size()), nullptr, 0, nullptr, nullptr);
    std::u8string u8str(sizeNeeded, u8'\0');
    WideCharToMultiByte(CP_UTF8, 0, wstr.data(), static_cast<int>(wstr.size()), reinterpret_cast<char*>(u8str.data()), sizeNeeded, nullptr, nullptr);

    return u8str;
}

std::wstring u8stringToWString(const std::u8string& u8str) {
    if (u8str.empty()) {
        return std::wstring();
    }

    int sizeNeeded = MultiByteToWideChar(CP_UTF8, 0, reinterpret_cast<const char*>(u8str.data()), static_cast<int>(u8str.size()), nullptr, 0);
    std::wstring wstr(sizeNeeded, L'\0');
    MultiByteToWideChar(CP_UTF8, 0, reinterpret_cast<const char*>(u8str.data()), static_cast<int>(u8str.size()), wstr.data(), sizeNeeded);

    return wstr;
}

std::u8string ToU8StringUnchecked(const std::string& str) {
    return { reinterpret_cast<const char8_t*>(str.data()), str.size() };
}

std::string ToStringUnchecked(const std::u8string& str) {
    return { reinterpret_cast<const char*>(str.data()), str.size() };
}

size_t GetUnicodeSpaceByteCount(std::u8string_view str, bool from_front) {
    if (str.empty()) return 0;

    if (from_front) {
        uint8_t lead = static_cast<uint8_t>(str[0]);

        if (lead == 0x20 || (lead >= 0x09 && lead <= 0x0D)) {
            return 1;
        }

        if (lead == 0xC2 && str.size() >= 2) {
            uint8_t b2 = static_cast<uint8_t>(str[1]);
            if (b2 == 0x85) return 2;
        }

        if (lead == 0xE2 && str.size() >= 3) {
            uint8_t b2 = static_cast<uint8_t>(str[1]);
            uint8_t b3 = static_cast<uint8_t>(str[2]);

            if (b2 == 0x80 && (b3 >= 0x80 && b3 <= 0x8A)) return 3;
            if (b2 == 0x80 && (b3 == 0xA8 || b3 == 0xA9)) return 3;
            if (b2 == 0x80 && b3 == 0xAF) return 3;
            if (b2 == 0x81 && b3 == 0x9F) return 3;
        }
        if (lead == 0xE3 && str.size() >= 3) {
            if (static_cast<uint8_t>(str[1]) == 0x80 && static_cast<uint8_t>(str[2]) == 0x80) {
                return 3;
            }
        }
    } else {
        size_t len = str.size();
        uint8_t last = static_cast<uint8_t>(str[len - 1]);

        if (last == 0x20 || (last >= 0x09 && last <= 0x0D)) {
            return 1;
        }

        if (len >= 2) {
            uint8_t b1 = static_cast<uint8_t>(str[len - 2]);
            if (b1 == 0xC2 && last == 0x85) return 2;
        }

        if (len >= 3) {
            uint8_t b1 = static_cast<uint8_t>(str[len - 3]);
            uint8_t b2 = static_cast<uint8_t>(str[len - 2]);

            if (b1 == 0xE2) {
                if (b2 == 0x80 && (last >= 0x80 && last <= 0x8A)) return 3;
                if (b2 == 0x80 && (last == 0xA8 || last == 0xA9)) return 3;
                if (b2 == 0x80 && last == 0xAF) return 3;
                if (b2 == 0x81 && last == 0x9F) return 3;
            }
            if (b1 == 0xE3 && b2 == 0x80 && last == 0x80) {
                return 3;
            }
        }
    }

    return 0;
}

std::u8string_view StringTrim(std::u8string_view input) {
    size_t first = 0;
    while (first < input.size()) {
        size_t space_bytes = GetUnicodeSpaceByteCount(input.substr(first), true);
        if (space_bytes == 0) break;
        first += space_bytes;
    }

    size_t last = input.size();
    while (last > first) {
        size_t space_bytes = GetUnicodeSpaceByteCount(input.substr(first, last - first), false);
        if (space_bytes == 0) break;
        last -= space_bytes;
    }

    return input.substr(first, last - first);
}

std::wstring StringLower(const std::wstring& input) {
    if (input.empty()) return {};
    int length = LCMapStringEx(LOCALE_NAME_INVARIANT, LCMAP_LOWERCASE, input.data(), static_cast<int>(input.size()), nullptr, 0, nullptr, nullptr, 0);

    if (length <= 0) return {};

    std::wstring lowered(length, L'\0');

    if (LCMapStringEx(LOCALE_NAME_INVARIANT, LCMAP_LOWERCASE, input.data(), static_cast<int>(input.size()), lowered.data(), length, nullptr, nullptr, 0) <= 0) {
        return {};
    }
    return lowered;
}

std::u8string StringLower(std::u8string_view input) {
    if (input.empty()) return {};

    std::wstring wide = u8stringToWString({ input.begin(), input.end() });

    return wstringToU8String(StringLower(wide));
}

bool StringEqualIgnoreCase(std::u8string_view a, std::u8string_view b) {
    std::wstring wa = u8stringToWString({ a.begin(), a.end() });
    std::wstring wb = u8stringToWString({ b.begin(), b.end() });

    return CompareStringOrdinal(wa.data(), static_cast<int>(wa.size()), wb.data(), static_cast<int>(wb.size()), TRUE) == CSTR_EQUAL;
}


std::istream& u8getline(std::istream& input, std::u8string& u8str, char8_t delim) {
    std::string temp;
    if (std::getline(input, temp, static_cast<char>(delim))) {
        u8str = std::u8string(reinterpret_cast<const char8_t*>(temp.data()), temp.size());
    }
    return input;
}

template <std::integral T> requires (!std::same_as<T, bool>)
bool StringToInt(std::string_view sv, T& out) {
    if (sv.empty()) return false;

    bool negative = false;
    if constexpr (std::is_signed_v<T>) {
        if (sv.starts_with('-')) {
            negative = true;
            sv.remove_prefix(1);
        } else if (sv.starts_with('+'))
        {
            sv.remove_prefix(1);
        }
    } else {
        if (sv.starts_with('-') || sv.starts_with('+')) return false;
    }

    if (sv.empty()) return false;

    int base = 10;

    if (sv.starts_with("0x") || sv.starts_with("0X")) {
        base = 16;
        sv.remove_prefix(2);
    } else if (sv.starts_with("0b") || sv.starts_with("0B"))
    {
        base = 2;
        sv.remove_prefix(2);

    } else if (sv.starts_with("0o") || sv.starts_with("0O")) {
        base = 8;
        sv.remove_prefix(2);
    }

    if (sv.empty()) return false;

    T value{};

    auto [ptr, ec] = std::from_chars(sv.data(), sv.data() + sv.size(), value, base);

    if (ec != std::errc{} || ptr != sv.data() + sv.size()) return false;

    if constexpr (std::is_signed_v<T>) { 
        out = negative ? -value : value;
    } else {
        out = value;
    }

    return true;
}

bool HexDigit32(char8_t c, uint32_t& value) {
    if (c >= u8'0' && c <= u8'9') {
        value = c - L'0';
        return true;
    }

    if (c >= u8'a' && c <= u8'f') {
        value = c - L'a' + 10;
        return true;
    }

    if (c >= u8'A' && c <= u8'F') {
        value = c - u8'A' + 10;
        return true;
    }

    return false;
}

bool ParseHex32(const std::u8string& str, size_t& pos, int32_t count, uint32_t& value) {
    value = 0;

    for (int i = 0; i < count; ++i) {
        if (pos >= str.length()) return false;

        uint32_t digit;

        if (!HexDigit32(str[pos], digit)) return false;

        value = (value << 4) | digit;
        ++pos;
    }

    return true;
}

bool ParseStringArg(const std::u8string& args, std::u8string& out) {
    if (args.length() < 2) return false;

    const wchar_t quote = args.front();

    if (quote != u8'"' && quote != u8'\'')
        return false;

    if (args.back() != quote)
        return false;

    const std::u8string value =
        args.substr(1, args.length() - 2);

    // literal string
    if (quote == u8'\'') {
        out = value;
        return true;
    }

    out.clear();
    out.reserve(value.length());

    size_t pos = 0;

    while (pos < value.length())
    {
        wchar_t c = value[pos++];

        if (c != u8'\\')
        {
            out += c;
            continue;
        }

        if (pos >= value.length())
            return false;

        wchar_t escaped = value[pos++];

        switch (escaped)
        {
        case u8'\'':
            out += u8'\'';
            break;

        case u8'"':
            out += u8'"';
            break;

        case u8'\\':
            out += u8'\\';
            break;

        case u8'0':
            out += u8'\0';
            break;

        case u8'a':
            out += u8'\a';
            break;

        case u8'b':
            out += u8'\b';
            break;

        case u8'e':
            out += u8'\u001B';
            break;

        case u8'f':
            out += u8'\f';
            break;

        case u8'n':
            out += u8'\n';
            break;

        case u8'r':
            out += u8'\r';
            break;

        case u8't':
            out += u8'\t';
            break;

        case u8'v':
            out += u8'\v';
            break;

        case u8'x':
        {
            unsigned int code;

            if (!ParseHex32(value, pos, 2, code))
                return false;

            out += static_cast<wchar_t>(code);
            break;
        }

        case u8'u':
        {
            unsigned int code;

            if (!ParseHex32(value, pos, 4, code))
                return false;

            out += static_cast<wchar_t>(code);
            break;
        }

        case u8'U':
        {
            unsigned int code;

            if (!ParseHex32(value, pos, 8, code))
                return false;

            if (code > 0x10FFFF)
                return false;

            // UTF-16 surrogate pair
            if (code > 0xFFFF) {
                code -= 0x10000;
                wchar_t high = static_cast<wchar_t>(0xD800 + (code >> 10));
                wchar_t low = static_cast<wchar_t>(0xDC00 + (code & 0x3FF));
                out += high;
                out += low;
            } else {
                // Reject isolated surrogate code points
                if (code >= 0xD800 && code <= 0xDFFF)
                    return false;

                out += static_cast<wchar_t>(code);
            }
            break;
        }

        default:
            return false;
        }
    }

    return true;
}

namespace InputSim {
    BOOL CALLBACK EnumWindowsCallback(HWND hWnd, LPARAM lParam) {
        if (!IsWindowVisible(hWnd)) {
            return TRUE; // Continue searching
        }

        int length = GetWindowTextLengthW(hWnd);
        if (length == 0) {
            return TRUE; // Continue searching
        }

        std::wstring title(length, L'\0');
        GetWindowTextW(hWnd, &title[0], length + 1);

        auto* data = reinterpret_cast<WindowSearchData*>(lParam);

        if (title == data->targetName) {
            data->foundHwnd = hWnd;
            return FALSE;
        }

        return TRUE;
    }

    HWND FindVisibleWindows(const std::wstring& name) {
        WindowSearchData data;
        data.targetName = name;
        data.foundHwnd = nullptr;

        EnumWindows(EnumWindowsCallback, reinterpret_cast<LPARAM>(&data));

        return data.foundHwnd;
    }

    bool Delay(const std::atomic<bool>& isForceStopRequested, int64_t ms) {
        std::mutex mutex;
        std::condition_variable_any cv;
        std::unique_lock lock(mutex);

        cv.wait_for(
            lock,
            std::chrono::milliseconds(ms),
            [&isForceStopRequested] {
                return isForceStopRequested.load();
            }
        );
        return !isForceStopRequested.load();
    }

    void MouseDown(MouseButtonType type) {
        INPUT input = { .type = INPUT_MOUSE };
        switch (type) {
        case MouseButtonType::Left:
            input.mi.dwFlags = MOUSEEVENTF_LEFTDOWN;
            break;
        case MouseButtonType::Right:
            input.mi.dwFlags = MOUSEEVENTF_RIGHTDOWN;
            break;
        case MouseButtonType::Middle:
            input.mi.dwFlags = MOUSEEVENTF_MIDDLEDOWN;
            break;
        case MouseButtonType::X1:
            input.mi.dwFlags = MOUSEEVENTF_XDOWN; 
            input.mi.mouseData = XBUTTON1;
            break;
        case MouseButtonType::X2:
            input.mi.dwFlags = MOUSEEVENTF_XDOWN;
            input.mi.mouseData = XBUTTON2;
            break;
        default:
            return;
        }
        SendInput(1, &input, sizeof(INPUT));
    }

    void MouseUp(MouseButtonType type) {
        INPUT input = { .type = INPUT_MOUSE };
        switch (type) {
        case MouseButtonType::Left:
            input.mi.dwFlags = MOUSEEVENTF_LEFTUP;
            break;
        case MouseButtonType::Right:
            input.mi.dwFlags = MOUSEEVENTF_RIGHTUP;
            break;
        case MouseButtonType::Middle:
            input.mi.dwFlags = MOUSEEVENTF_MIDDLEUP;
            break;
        case MouseButtonType::X1:
            input.mi.dwFlags = MOUSEEVENTF_XUP;
            input.mi.mouseData = XBUTTON1;
            break;
        case MouseButtonType::X2:
            input.mi.dwFlags = MOUSEEVENTF_XUP;
            input.mi.mouseData = XBUTTON2;
            break;
        default:
            return;
        }
        SendInput(1, &input, sizeof(INPUT));
    }

    void MouseAbsoluteMove(int32_t x, int32_t y) {
        int screenW = GetSystemMetrics(SM_CXSCREEN);
        int screenH = GetSystemMetrics(SM_CYSCREEN);
        INPUT input = {
            .type = INPUT_MOUSE,
            .mi = {
                .dx = (int32_t)((x * 65535.0) / screenW),
                .dy = (int32_t)((y * 65535.0) / screenH),
                .dwFlags = MOUSEEVENTF_ABSOLUTE | MOUSEEVENTF_MOVE
            }
        };
        SendInput(1, &input, sizeof(INPUT));
    }

    void MouseRelativeMove(int32_t dx, int32_t dy) {
        INPUT input = {
            .type = INPUT_MOUSE,
            .mi = {.dx = dx, .dy = dy, .dwFlags = MOUSEEVENTF_MOVE }
        };
        SendInput(1, &input, sizeof(INPUT));
    }

    void Wheel(bool up) {
        INPUT input = {
            .type = INPUT_MOUSE,
            .mi = {.mouseData = static_cast<DWORD>(up ? WHEEL_DELTA : -WHEEL_DELTA), .dwFlags = MOUSEEVENTF_WHEEL }
        };
        SendInput(1, &input, sizeof(INPUT));
    }

    void HoldTitleBar(const std::wstring& windowName) {
        POINT originalMousePos;
        ::GetCursorPos(&originalMousePos);
        auto hWnd = FindVisibleWindows(windowName);
        ::PostMessageW(hWnd, WM_SYSCOMMAND, SC_MOVE, NULL);
        std::this_thread::sleep_for(std::chrono::duration<int32_t, std::milli>(50));
        SetCursorPos(originalMousePos.x, originalMousePos.y);
    }

    void ReleaseTitleBar(const std::wstring& windowName) {
        auto hWnd = FindVisibleWindows(windowName);
        ::PostMessageW(hWnd, WM_CANCELMODE, NULL, NULL);
    }

    void ShowCoordinate() {
        POINT pt;
        ::GetCursorPos(&pt); 
#ifdef _LANG_ZH_TW_
        ::MessageBoxW(nullptr, std::format(L"X: {}, Y: {}", pt.x, pt.y).c_str(), L"現在座標", MB_ICONINFORMATION | MB_OK);
#else
        ::MessageBoxW(nullptr, std::format(L"X: {}, Y: {}", pt.x, pt.y).c_str(), L"Current Coordinate", MB_ICONINFORMATION | MB_OK);
#endif // _LANG_ZH_TW_
    }

    bool IsExtendedKey(uint16_t vkCode) {
        switch (vkCode) {
        case VK_RMENU: case VK_RCONTROL:
        case VK_INSERT: case VK_DELETE:
        case VK_HOME: case VK_END:
        case VK_PRIOR: case VK_NEXT:
        case VK_UP: case VK_DOWN: case VK_LEFT: case VK_RIGHT:
            return true;
        default:
            return false;
        }
    }

    void KeyboardDown(uint16_t vkCode) {
        INPUT input = { .type = INPUT_KEYBOARD, .ki = {.wVk = vkCode } };
        if (IsExtendedKey(vkCode)) {
            input.ki.dwFlags |= KEYEVENTF_EXTENDEDKEY;
        }
        input.ki.wScan = MapVirtualKeyW(vkCode, MAPVK_VK_TO_VSC);
        input.ki.dwFlags |= KEYEVENTF_SCANCODE;
        SendInput(1, &input, sizeof(INPUT));
    }

    void KeyboardUp(uint16_t vkCode) {
        INPUT input = { .type = INPUT_KEYBOARD, .ki = {.wVk = vkCode, .dwFlags = KEYEVENTF_KEYUP } };
        if (IsExtendedKey(vkCode)) {
            input.ki.dwFlags |= KEYEVENTF_EXTENDEDKEY;
        }
        input.ki.wScan = MapVirtualKeyW(vkCode, MAPVK_VK_TO_VSC);
        input.ki.dwFlags |= KEYEVENTF_SCANCODE;
        SendInput(1, &input, sizeof(INPUT));
    }

    void OutputText(const std::atomic<bool>& isForceStopRequested, const std::wstring& text, int64_t ms) {
        for (const auto& c : text) {
            if (c == L'\0') continue;
            INPUT inputs[2] = {};
            inputs[0].type = INPUT_KEYBOARD;
            inputs[0].ki.wScan = c;
            inputs[0].ki.dwFlags = KEYEVENTF_UNICODE;

            inputs[1] = inputs[0];
            inputs[1].ki.dwFlags = KEYEVENTF_UNICODE | KEYEVENTF_KEYUP;

            SendInput(2, inputs, sizeof(INPUT));
            if (isForceStopRequested.load()) break;
            if (ms > 0 && !Delay(isForceStopRequested, ms)) break;
        }
    }
}

std::optional<MouseButtonType> MouseButtonTypeFromString(const std::u8string& str)
{
    if (str == u8"left" || str == u8"左") return MouseButtonType::Left;
    else if (str == u8"right" || str == u8"右")  return MouseButtonType::Right;
    else if (str == u8"middle" || str == u8"中") return MouseButtonType::Middle;
    else if (str == u8"x1") return MouseButtonType::X1;
    else if (str == u8"x2") return MouseButtonType::X2;
    return std::nullopt;
}

std::optional<ScriptRunMode> ScriptRunModeFromString(const std::u8string& str)
{
    if (str == u8"fullsingle" || str == u8"完整單次") return ScriptRunMode::FullSingle;
    else if (str == u8"fullcontinuous" || str == u8"完整連續")  return ScriptRunMode::FullContinuous;
    else if (str == u8"fullswitch" || str == u8"完整切換") return ScriptRunMode::FullSwitch;
    if (str == u8"single" || str == u8"單次") return ScriptRunMode::Single;
    else if (str == u8"continuous" || str == u8"連續")  return ScriptRunMode::Continuous;
    else if (str == u8"switch" || str == u8"切換") return ScriptRunMode::Switch;
    return std::nullopt;
}

std::u8string ScriptRunModeToString(ScriptRunMode mode) {
#ifdef _LANG_ZH_TW_
    switch (mode) {
    case ScriptRunMode::FullSingle:
        return u8"完整單次";
    case ScriptRunMode::FullContinuous:
        return u8"完整連續";
    case ScriptRunMode::FullSwitch:
        return u8"完整切換";
    case ScriptRunMode::Single:
        return u8"單次";
    case ScriptRunMode::Continuous:
        return u8"連續";
    case ScriptRunMode::Switch:
        return u8"切換";
    }
#else
    switch (mode) {
    case ScriptRunMode::FullSingle:
        return u8"FullSingle";
    case ScriptRunMode::FullContinuous:
        return u8"FullContinuous";
    case ScriptRunMode::FullSwitch:
        return u8"FullSwitch";
    case ScriptRunMode::Single:
        return u8"Single";
    case ScriptRunMode::Continuous:
        return u8"Continuous";
    case ScriptRunMode::Switch:
        return u8"Switch";
    }
#endif // _LANG_ZH_TW_

    return u8"";
}

/// <summary>
/// Any: modifier state doesn't matter
/// MustDown: at least one side is down
/// MustUp: both sides are up
/// </summary>
bool CheckKeyModifier(bool leftDown, bool rightDown, KeyModifierRequirement requirement) {
    const bool isDown = leftDown || rightDown;
    switch (requirement)
    {
    case KeyModifierRequirement::Any:
        return true;

    case KeyModifierRequirement::MustDown:
        return isDown;

    case KeyModifierRequirement::MustUp:
        return !isDown;
    }

    return false;
}

std::u8string HotkeyDataToString(const HotkeyData& data) {
    std::u8string result;

    auto appendModifier = [&result](KeyModifierRequirement requirement, std::u8string_view downName, std::u8string_view upName) {
        if (requirement == KeyModifierRequirement::MustDown) {
            if (!result.empty()) result += u8"+";
            result += downName;
        } else if (requirement == KeyModifierRequirement::MustUp) {
            if (!result.empty()) result += u8"+";
            result += upName;
        }
    };

    appendModifier(data.ctrl, u8"CtrlDown", u8"CtrlUp");
    appendModifier(data.shift, u8"ShiftDown", u8"ShiftUp");
    appendModifier(data.alt, u8"AltDown", u8"AltUp");

    if (!result.empty()) result += u8"+";

    auto keyName = GetNameByKey(data.key);

    if (keyName.has_value()) {
        result += keyName.value();
    } else {
        std::string hexText = std::format("0x{:X}", data.key);
        std::u8string u8HexText = std::u8string(hexText.begin(), hexText.end());
        result += u8HexText;
    }

    return result;
}

bool CheckIsHotkeyDown(const HotkeyData& hotkeyData, const KeyboardEventData& eventData) {
    return eventData.isDown && eventData.vkCode == hotkeyData.key &&
        CheckKeyModifier(eventData.isLCtrlDown, eventData.isRCtrlDown, hotkeyData.ctrl) &&
        CheckKeyModifier(eventData.isLShiftDown, eventData.isRShiftDown, hotkeyData.shift) &&
        CheckKeyModifier(eventData.isLAltDown, eventData.isRAltDown, hotkeyData.alt);
}

bool ParseHotkeyModifier(std::u8string_view value, HotkeyData& out) {
    if (value == u8"ctrldown") {
        out.ctrl = KeyModifierRequirement::MustDown;
        return true;
    } else if (value == u8"ctrlup") {
        out.ctrl = KeyModifierRequirement::MustUp;
        return true;
    } else if (value == u8"shiftdown") {
        out.shift = KeyModifierRequirement::MustDown;
        return true;
    } else if (value == u8"shiftup") {
        out.shift = KeyModifierRequirement::MustUp;
        return true;
    } else if (value == u8"altdown") {
        out.alt = KeyModifierRequirement::MustDown;
        return true;
    } else if (value == u8"altup") {
        out.alt = KeyModifierRequirement::MustUp;
        return true;
    }

    return false;
}

bool ParseHotkeyData(std::u8string_view args, HotkeyData& out) {
    const size_t comma = args.find(L',');

    std::u8string_view keyArg;
    std::u8string_view modifierArgs;

    if (comma == std::u8string_view::npos) {
        keyArg = args;
    } else {
        keyArg = args.substr(0, comma);
        modifierArgs = args.substr(comma + 1);
    }

    keyArg = StringTrim(keyArg);

    if (keyArg.empty()) return false;

    if (auto key = GetKeyByName(keyArg); key.has_value()) {
        out.key = static_cast<int32_t>(key.value());
    } else {
        auto stringKey = std::string(keyArg.begin(), keyArg.end());

        int32_t vkCode = 0;

        if (!StringToInt<int32_t>(stringKey, vkCode)) return false;

        out.key = vkCode;
    }

    size_t start = 0;
    while (start < modifierArgs.size()) {
        size_t end = modifierArgs.find(L',', start);

        if (end == std::wstring_view::npos) end = modifierArgs.size();

        auto modifier = StringTrim(modifierArgs.substr(start, end - start));

        if (modifier.empty()) return false;

        if (!ParseHotkeyModifier(modifier, out))return false;

        start = end + 1;
    }

    return true;
}


bool ParseScriptLine(const std::u8string& line, ParsedCommand& outCmd, std::u8string& outError) {
    outError.clear();

    if (line.length() <= 0) {
        return true;
    }

    size_t openParen = line.find('(');
    size_t closeParen = line.rfind(')');
    if (openParen == std::u8string::npos || closeParen == std::u8string::npos || closeParen < openParen) {
#ifdef _LANG_ZH_TW_
        outError = u8"缺少或無效的括號 '()'";
#else
        outError = u8"Missing or invalid parentheses '()'";
#endif
        return false;
    }

    if (line.size() - 1 != closeParen) {
#ifdef _LANG_ZH_TW_
        outError = u8"括號 ')' 後面不應該有其他文字，如有需要可以將文字加在 # 之後;";
#else
        outError = u8"Trailing characters found after closing parenthesis ')', If needed, you can add text after #.";
#endif
        return false;
    }

    std::u8string func = std::u8string(StringTrim(StringLower(std::u8string_view(line).substr(0, openParen))));
    std::u8string args = std::u8string(StringTrim(std::u8string_view(line).substr(openParen + 1, closeParen - openParen - 1)));

    try {
        if (func == u8"setstartkey" || func == u8"setendkey" || func == u8"設定開始按鍵" || func == u8"設定停止按鍵") {
            outCmd.type = (func == u8"setstartkey" || func == u8"設定開始按鍵") ? CmdType::SetStartKey : CmdType::SetEndKey;
            HotkeyData hotkey;
            if (!ParseHotkeyData(StringLower(args), hotkey)) {
#ifdef _LANG_ZH_TW_
                outError = u8"無效的按鍵格式: " + args;
#else
                outError = u8"Invalid hotkey format: " + args;
#endif
                return false;
            }
            outCmd.args = hotkey;
            return true;
        }
        else if (func == u8"setrunmode" || func == u8"設定運行模式") {
            auto runMode = ScriptRunModeFromString(StringLower(args));
            if (!runMode.has_value()) {
#ifdef _LANG_ZH_TW_
                outError = u8"無效的運行模式引數: " + args;
#else
                outError = u8"Invalid run mode parameter: " + args;
#endif
                return false;
            }
            outCmd.type = CmdType::SetScriptRunMode;
            outCmd.args = runMode.value();
            return true;
        }
        else if (func == u8"delay" || func == u8"延遲") {
            outCmd.type = CmdType::Delay;
            int64_t num;
            if (!StringToInt<int64_t>(std::string(args.begin(), args.end()), num)) {
#ifdef _LANG_ZH_TW_
                outError = u8"無效的延遲引數 (必須為整數): " + args;
#else
                outError = u8"Invalid delay parameter (must be an integer): " + args;
#endif
                return false;
            }
            outCmd.args = num;
            return true;
        }
        else if (func == u8"mousedown" || func == u8"mouseup" || func == u8"滑鼠按下" || func == u8"滑鼠放開") {
            auto type = MouseButtonTypeFromString(StringLower(args));
            if (!type.has_value()) {
#ifdef _LANG_ZH_TW_
                outError = u8"無效的滑鼠按鍵引數: " + args;
#else
                outError = u8"Invalid mouse button parameter: " + args;
#endif
                return false;
            }
            outCmd.type = (func == u8"mousedown" || func == u8"滑鼠按下") ? CmdType::MouseDown : CmdType::MouseUp;
            outCmd.args = type.value();
            return true;
        }
        else if (func == u8"absolutemove" || func == u8"relativemove" || func == u8"絕對移動" || func == u8"相對移動") {
            auto splitView = args | std::views::split(u8',');

            std::vector<int32_t> splitArgs;

            for (auto&& part : splitView) {
                int32_t num;
                std::u8string s(part.begin(), part.end());
                s = StringTrim(s);
                if (!StringToInt<int32_t>(std::string(s.begin(), s.end()), num)) {
#ifdef _LANG_ZH_TW_
                    outError = u8"無效的座標引數 (必須為整數): " + args;
#else
                    outError = u8"Invalid coordinate parameter (must be integers): " + args;
#endif
                    return false;
                }
                splitArgs.push_back(num);
            }

            if (splitArgs.size() != 2) {
#ifdef _LANG_ZH_TW_
                outError = u8"移動指令只需要2個引數 (x, y): " + args;
#else
                outError = u8"Move command expects exactly 2 parameters (x, y): " + args;
#endif
                return false;
            }

            outCmd.type = (func == u8"absolutemove" || func == u8"絕對移動") ? CmdType::MouseAbsoluteMove : CmdType::MouseRelativeMove;
            outCmd.args = std::move(splitArgs);
            return true;
        }
        else if (func == u8"keyboarddown" || func == u8"keyboardup" || func == u8"鍵盤按下" || func == u8"鍵盤放開") {
            auto key = GetKeyByName(args);
            outCmd.type = (func == u8"keyboarddown" || func == u8"鍵盤按下") ? CmdType::KeyboardDown : CmdType::KeyboardUp;
            if (key.has_value() && key > 0 && key < 65536) {
                outCmd.args = (uint16_t)key.value();
                return true;
            }
            uint16_t num = 0;
            if (!StringToInt<uint16_t>(std::string(args.begin(), args.end()), num)) {
#ifdef _LANG_ZH_TW_
                outError = u8"無效的按鍵名稱或按鍵編號: " + args;
#else
                outError = u8"Invalid key parameter or virtual key code: " + args;
#endif
                return false;
            }
            outCmd.args = (uint16_t)num;
            return true;
        }
        else if (func == u8"wheeldown" || func == u8"wheelup" || func == u8"滾輪下" || func == u8"滾輪上") {
            if (!args.empty()) {
#ifdef _LANG_ZH_TW_
                outError = u8"滾輪指令不接收引數: " + args;
#else
                outError = u8"Wheel command does not accept parameters: " + args;
#endif
                return false;
            }
            outCmd.type = (func == u8"wheeldown" || func == u8"滾輪下") ? CmdType::WheelDown : CmdType::WheelUp;
            return true;
        }
        else if (func == u8"holdtitlebar" || func == u8"releasetitlebar" || func == u8"按住視窗標題" || func == u8"放開視窗標題") {
            std::u8string text;
            if (!ParseStringArg(args, text)) {
#ifdef _LANG_ZH_TW_
                outError = u8"無效的文字格式（文字應該被\"\"或''包圍）: " + args;
#else
                outError = u8"Invalid string format (expected quoted string): " + args;
#endif
                return false;
            }
            outCmd.type = (func == u8"holdtitlebar" || func == u8"按住視窗標題") ? CmdType::HoldTitleBar : CmdType::ReleaseTitleBar;
            outCmd.args = u8stringToWString(text);
            return true;
        }
        else if (func == u8"outputtext" || func == u8"輸出文字") {
            size_t startQuote = args.find_first_of(u8"\"'");
            if (startQuote == std::u8string::npos) {
#ifdef _LANG_ZH_TW_
                outError = u8"無效的文字格式（文字應該被\"\"或''包圍）: " + args;
#else
                outError = u8"Invalid string format (expected quoted string): " + args;
#endif
                return false;
            }

            wchar_t quote = args[startQuote];
            size_t endQuote = args.find_last_of(quote);
            if (endQuote == std::u8string::npos) {
#ifdef _LANG_ZH_TW_
                outError = u8"無效的文字格式(未閉合的引號): " + args;
#else
                outError = u8"Invalid string format (unclosed quote): " + args;
#endif
                return false;
            }

            std::u8string stringPart = args.substr(0, endQuote + 1);
            std::u8string_view remaining = StringTrim(std::u8string_view(args).substr(endQuote + 1));

            std::u8string text;
            if (!ParseStringArg(stringPart, text)) {
#ifdef _LANG_ZH_TW_
                outError = u8"無效的文字格式（文字應該被\"\"或''包圍）: " + args;
#else
                outError = u8"Invalid string format (expected quoted string): " + args;
#endif
                return false;
            }

            int64_t delayMs = 0;
            if (!remaining.empty()) {
                if (remaining.front() == L',') {
                    std::u8string delayStr = std::u8string(StringTrim(remaining.substr(1)));
                    if (!StringToInt<int64_t>(std::string(delayStr.begin(), delayStr.end()), delayMs)) {
#ifdef _LANG_ZH_TW_
                        outError = u8"無效的延遲引數 (必須為整數): " + args;
#else
                        outError = u8"Invalid delay parameter (must be an integer): " + args;
#endif
                        return false;
                    }
                }
                else {
#ifdef _LANG_ZH_TW_
                    outError = u8"無效的引數格式: " + args;
#else
                    outError = u8"Invalid parameter format: " + args;
#endif
                    return false;
                }
            }

            outCmd.type = CmdType::OutputText;
            outCmd.args = OutputTextData{ u8stringToWString(text), delayMs };
            return true;
            }
        else if (func == u8"showcoordinate" || func == u8"顯示座標") {
            if (!args.empty()) {
#ifdef _LANG_ZH_TW_
                outError = u8"顯示座標指令不接收引數: " + args;
#else
                outError = u8"ShowCoordinate command does not accept parameters: " + args;
#endif
                return false;
            }
            outCmd.type = CmdType::ShowCoordinate;
            return true;
        }
    }
    catch (...) {
#ifdef _LANG_ZH_TW_
        outError = u8"解析該行時發生未知的例外狀況。";
#else
        outError = u8"An unexpected exception occurred during line parsing.";
#endif
        return false;
    }

#ifdef _LANG_ZH_TW_
    outError = u8"未知的指令: " + func;
#else
    outError = u8"Unknown function: " + func;
#endif
    return false;
}

bool ValidateAndParseScript(
    const std::vector<std::u8string>& lines,
    HotkeyData& outStartKey,
    HotkeyData& outEndKey,
    ScriptRunMode& outRunMode,
    std::vector<ParsedCommand>& outParsed, 
    std::u8string& outErrorMsg, 
    int& outErrorLine
) {
    outParsed.clear();
    int lineNumber = 0;

    HotkeyData startKey;
    HotkeyData endKey;
    std::optional<ScriptRunMode> scriptRunMode = std::nullopt;

    for (const auto& rawLine : lines) {
        lineNumber++;

        std::u8string line = std::u8string(StringTrim(rawLine));
        size_t commnentPos = line.find('#');
        if (commnentPos != std::string::npos) {
            line = std::u8string(StringTrim(line.substr(0, commnentPos)));
        }

        if (line.empty()) {
            continue;
        }

        ParsedCommand cmd;
        std::u8string outError;
        if (!ParseScriptLine(line, cmd, outError)) {
            outErrorLine = lineNumber;
#ifdef _LANG_ZH_TW_
            outErrorMsg = wstringToU8String(std::format(L"腳本錯誤\r\n{}\r\n行數: {}:\r\n\"{}\"", u8stringToWString(outError), lineNumber, u8stringToWString(rawLine)));
#else
            outErrorMsg = wstringToU8String(std::format(L"Syntax Error\r\n{}\r\nLine {}:\r\n\"{}\"", u8stringToWString(outError), lineNumber, u8stringToWString(rawLine)));
#endif // _LANG_ZH_TW_
            return false;
        }

        if (cmd.type == CmdType::SetStartKey) {
            if (startKey.key != 0) {
                outErrorLine = lineNumber;
#ifdef _LANG_ZH_TW_
                outErrorMsg = wstringToU8String(std::format(L"腳本錯誤\r\n已經設定了開始按鍵\r\n行數: {}:\r\n\"{}\"\r\n", lineNumber, u8stringToWString(rawLine)));
#else
                outErrorMsg = wstringToU8String(std::format(L"Syntax Error\r\nStart Key already defined\r\nLine {}:\r\n\"{}\"", lineNumber, u8stringToWString(rawLine)));
#endif // _LANG_ZH_TW_
                return false;
            }
            startKey = std::any_cast<HotkeyData>(cmd.args);
        } else if (cmd.type == CmdType::SetEndKey) {
            if (endKey.key != 0) {
                outErrorLine = lineNumber;
#ifdef _LANG_ZH_TW_
                outErrorMsg = wstringToU8String(std::format(L"腳本錯誤，行數: {}:\r\n\"{}\"\r\n已經設定了停止按鍵", lineNumber, u8stringToWString(rawLine)));
#else
                outErrorMsg = wstringToU8String(std::format(L"Syntax Error\r\nEnd Key already defined\r\nLine {}:\r\n\"{}\"", lineNumber, u8stringToWString(rawLine)));
#endif // _LANG_ZH_TW_
                return false;
            }
            endKey = std::any_cast<HotkeyData>(cmd.args);
        } else if (cmd.type == CmdType::SetScriptRunMode) {
            if (scriptRunMode.has_value()) {
                outErrorLine = lineNumber;
#ifdef _LANG_ZH_TW_
                outErrorMsg = wstringToU8String(std::format(L"腳本錯誤，行數: {}:\r\n\"{}\"\r\n已經設定了運行模式", lineNumber, u8stringToWString(rawLine)));
#else
                outErrorMsg = wstringToU8String(std::format(L"Syntax Error\r\nRun Mode already defined\r\nLine {}:\r\n\"{}\"", lineNumber, u8stringToWString(rawLine)));
#endif // _LANG_ZH_TW_
                return false;
            }
            scriptRunMode = std::any_cast<ScriptRunMode>(cmd.args);
        } else {
            outParsed.push_back(cmd);
        }
    }
    outStartKey = startKey;
    outEndKey = endKey;
    outRunMode = scriptRunMode.has_value() ? scriptRunMode.value() : ScriptRunMode::FullSingle;
    return true;
}

void ExecuteParsedCommand(const std::atomic<bool>& isForceStopRequested, const ParsedCommand& cmd) {
    switch (cmd.type) {
    case CmdType::Delay:
        InputSim::Delay(isForceStopRequested, std::any_cast<int64_t>(cmd.args));
        break;
    case CmdType::MouseDown:
        InputSim::MouseDown(std::any_cast<MouseButtonType>(cmd.args));
        break;
    case CmdType::MouseUp:
        InputSim::MouseUp(std::any_cast<MouseButtonType>(cmd.args));
        break;
    case CmdType::MouseAbsoluteMove: {
        const auto& pt = std::any_cast<const std::vector<int32_t>&>(cmd.args);
        InputSim::MouseAbsoluteMove(pt[0], pt[1]);
        break;
    }
    case CmdType::MouseRelativeMove: {
        const auto& pt = std::any_cast<const std::vector<int32_t>&>(cmd.args);
        InputSim::MouseRelativeMove(pt[0], pt[1]);
        break;
    }
    case CmdType::KeyboardDown:
        InputSim::KeyboardDown(std::any_cast<uint16_t>(cmd.args));
        break;
    case CmdType::KeyboardUp:
        InputSim::KeyboardUp(std::any_cast<uint16_t>(cmd.args));
        break;
    case CmdType::WheelUp:
        InputSim::Wheel(true);
        break;
    case CmdType::WheelDown:
        InputSim::Wheel(false);
        break;
    case CmdType::HoldTitleBar:
        InputSim::HoldTitleBar(std::any_cast<std::wstring>(cmd.args));
        break;
    case CmdType::ReleaseTitleBar:
        InputSim::ReleaseTitleBar(std::any_cast<std::wstring>(cmd.args));
        break;
    case CmdType::OutputText: {
        const auto& data = std::any_cast<const OutputTextData&>(cmd.args);
        InputSim::OutputText(isForceStopRequested, data.text, data.delayMs);
        break;
    }
    case CmdType::ShowCoordinate:
        InputSim::ShowCoordinate();
        break;
    }
}

void ExecuteScriptWorker(std::stop_token stopToken, ScriptData& data) {
    data.isRunning = true;
    if (data.preExecute != nullptr) data.preExecute();
    do {
        for (const auto& cmd : data.commands) {
            if (data.isForceStopRequested) break;
            ExecuteParsedCommand(data.isForceStopRequested, cmd);
        }

        if (data.mode == ScriptRunMode::Single || data.mode == ScriptRunMode::FullSingle) break;

    } while (!stopToken.stop_requested() && !data.isForceStopRequested);

    data.isRunning = false;
    if (data.postExecute != nullptr) data.postExecute(false);
}

bool ReadUtf8FileLines(const std::u8string& filePath, std::vector<std::u8string>& outLines) {
    outLines.clear();
    std::ifstream file(std::filesystem::path(filePath), std::ios::in | std::ios::binary);
    if (!file.is_open()) return false;

    std::string content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    file.close();

    // UTF-8 BOM
    size_t startOffset = 0;
    if (content.size() >= 3 &&
        static_cast<unsigned char>(content[0]) == 0xEF &&
        static_cast<unsigned char>(content[1]) == 0xBB &&
        static_cast<unsigned char>(content[2]) == 0xBF) {
        startOffset = 3;
    }

    std::stringstream ss(content.substr(startOffset));
    std::string line;

    while (std::getline(ss, line)) {
        if (!line.empty() && line.back() == '\r') {
            line.pop_back();
        }
        outLines.emplace_back(line.begin(), line.end());
    }

    return true;
}

bool StartScriptExecution(ScriptData& data) {
    if (data.isRunning) {
        return false;
    }
    data.isForceStopRequested = false;
    data.executeThread = std::jthread(ExecuteScriptWorker, std::ref(data));
    return true;
}

bool StopScriptExecution(ScriptData& data, bool forceStop, bool waitScriptEnd) {
    if (!data.isRunning) {
        return false;
    }

    auto mode = data.mode;
    if (forceStop || mode == ScriptRunMode::Single || mode == ScriptRunMode::Continuous || mode == ScriptRunMode::Switch) {
        data.isForceStopRequested = true;
    }
    if (data.executeThread.joinable()) {
        data.executeThread.request_stop();
        if (waitScriptEnd) data.executeThread.join();
    }
    return true;
}

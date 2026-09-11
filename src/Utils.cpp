#include "pch.h"
#include "Utils.h"
#include <chrono>
#include <sstream>
#include <format>

std::optional<int32_t> GetKeyByName(std::wstring_view name) {
    for (const auto& [key, value] : NAME_MATCH_KEYS) {
        if (key.size() != name.size()) continue;

        if (_wcsnicmp(key.data(), name.data(), key.size()) == 0) {
            return value;
        }
    }
    return std::nullopt;
}

std::optional<std::wstring> GetNameByKey(int32_t vk) {
    for (const auto& [key, value] : NAME_MATCH_KEYS) {
        if (value == vk) {
            return key;
        }
    }
    return std::nullopt;
}

std::string_view StringTrim(std::string_view in) {
    auto checkFn = [](char c) {
        return std::isspace(c);
    };

    auto view = std::views::all(in)
        | std::views::drop_while(checkFn)
        | std::views::reverse
        | std::views::drop_while(checkFn)
        | std::views::reverse;

    if (view.empty()) return {};

    return { std::to_address(view.begin()), view.size() };
}

std::wstring_view StringTrim(std::wstring_view in) {
    auto checkFn = [](wchar_t c) {
        return std::iswspace(c);
        };

    auto view = std::views::all(in)
        | std::views::drop_while(checkFn)
        | std::views::reverse
        | std::views::drop_while(checkFn)
        | std::views::reverse;

    if (view.empty()) return {};

    return { std::to_address(view.begin()), view.size() };
}

std::string StringLower(const std::string& in) {
    std::string result = in;
    std::transform(
        result.begin(), result.end(), result.begin(),
        [](unsigned char c) { return std::tolower(c); }
    );
    return result;
}

std::wstring StringLower(const std::wstring& in) {
    std::wstring result = in;
    std::transform(
        result.begin(), result.end(), result.begin(),
        [](wchar_t c) { return std::towlower(c); }
    );
    return result;
}


std::wstring Utf8ToUtf16(const std::string& utf8Str) {
    if (utf8Str.empty()) return std::wstring();

    int sizeNeeded = MultiByteToWideChar(CP_UTF8, 0, utf8Str.data(), static_cast<int>(utf8Str.size()), nullptr, 0);
    if (sizeNeeded <= 0) return std::wstring();

    std::wstring utf16Str(sizeNeeded, L'\0');
    MultiByteToWideChar(CP_UTF8, 0, utf8Str.data(), static_cast<int>(utf8Str.size()), utf16Str.data(), sizeNeeded);

    return utf16Str;
}

std::string Utf16ToUtf8(const std::wstring& utf16Str) {
    if (utf16Str.empty()) return std::string();

    int sizeNeeded = WideCharToMultiByte(CP_UTF8, 0, utf16Str.data(), static_cast<int>(utf16Str.size()), nullptr, 0, nullptr, nullptr);
    if (sizeNeeded <= 0) return std::string();

    std::string utf8Str(sizeNeeded, '\0');
    WideCharToMultiByte(CP_UTF8, 0, utf16Str.data(), static_cast<int>(utf16Str.size()), utf8Str.data(), sizeNeeded, nullptr, nullptr);

    return utf8Str;
}

template <std::integral T> requires (!std::same_as<T, bool>)
bool StringToInt(std::string_view sv, T& out) {
    if (sv.empty()) return false;

    int base = 10;

    if (sv.starts_with("0x") || sv.starts_with("0X")) {
        base = 16;
        sv.remove_prefix(2);
    }
    else if (sv.starts_with("0b") || sv.starts_with("0B")) {
        base = 2;
        sv.remove_prefix(2);
    }
    else if (sv.starts_with("0o") || sv.starts_with("0O")) {
        base = 8;
        sv.remove_prefix(2);
    }

    if (sv.empty()) return false;

    auto [ptr, ec] = std::from_chars(sv.data(), sv.data() + sv.size(), out, base);
    return (ec == std::errc{} && ptr == sv.data() + sv.size());
}

bool HexDigit32(wchar_t c, uint32_t& value) {
    if (c >= L'0' && c <= L'9') {
        value = c - L'0';
        return true;
    }

    if (c >= L'a' && c <= L'f') {
        value = c - L'a' + 10;
        return true;
    }

    if (c >= L'A' && c <= L'F') {
        value = c - L'A' + 10;
        return true;
    }

    return false;
}

bool ParseHex32(const std::wstring& str, size_t& pos, int32_t count, uint32_t& value) {
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

bool ParseStringArg(const std::wstring& args, std::wstring& out)
{
    if (args.length() < 2) return false;

    const wchar_t quote = args.front();

    if (quote != L'"' && quote != L'\'')
        return false;

    if (args.back() != quote)
        return false;

    const std::wstring value =
        args.substr(1, args.length() - 2);

    // literal string
    if (quote == L'\'') {
        out = value;
        return true;
    }

    out.clear();
    out.reserve(value.length());

    size_t pos = 0;

    while (pos < value.length())
    {
        wchar_t c = value[pos++];

        if (c != L'\\')
        {
            out += c;
            continue;
        }

        if (pos >= value.length())
            return false;

        wchar_t escaped = value[pos++];

        switch (escaped)
        {
        case L'\'':
            out += L'\'';
            break;

        case L'"':
            out += L'"';
            break;

        case L'\\':
            out += L'\\';
            break;

        case L'0':
            out += L'\0';
            break;

        case L'a':
            out += L'\a';
            break;

        case L'b':
            out += L'\b';
            break;

        case L'e':
            out += L'\u001B';
            break;

        case L'f':
            out += L'\f';
            break;

        case L'n':
            out += L'\n';
            break;

        case L'r':
            out += L'\r';
            break;

        case L't':
            out += L'\t';
            break;

        case L'v':
            out += L'\v';
            break;

        case L'x':
        {
            unsigned int code;

            if (!ParseHex32(value, pos, 2, code))
                return false;

            out += static_cast<wchar_t>(code);
            break;
        }

        case L'u':
        {
            unsigned int code;

            if (!ParseHex32(value, pos, 4, code))
                return false;

            out += static_cast<wchar_t>(code);
            break;
        }

        case L'U':
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
    bool Delay(std::stop_token stopToken, int64_t ms) {
        std::mutex mutex;
        std::condition_variable_any cv;
        std::unique_lock lock(mutex);

        cv.wait_for(
            lock,
            std::chrono::milliseconds(ms),
            [&stopToken] {
                return stopToken.stop_requested();
            }
        );
        return !stopToken.stop_requested();
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

    void ShowCoordinate() {
        CPoint pt;
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

    void OutputText(std::stop_token stopToken, const std::wstring& text, int64_t ms) {
        for (const auto& c : text) {
            if (c == L'\0') continue;
            INPUT inputs[2] = {};
            inputs[0].type = INPUT_KEYBOARD;
            inputs[0].ki.wScan = c;
            inputs[0].ki.dwFlags = KEYEVENTF_UNICODE;

            inputs[1] = inputs[0];
            inputs[1].ki.dwFlags = KEYEVENTF_UNICODE | KEYEVENTF_KEYUP;

            SendInput(2, inputs, sizeof(INPUT));
            if (stopToken.stop_requested()) break;
            if (ms > 0 && !Delay(stopToken, ms)) break;
        }
    }
}

std::optional<MouseButtonType> MouseButtonTypeFromString(const std::wstring& str)
{
    if (str == L"left" || str == L"左") return MouseButtonType::Left;
    else if (str == L"right" || str == L"右")  return MouseButtonType::Right;
    else if (str == L"middle" || str == L"中") return MouseButtonType::Middle;
    else if (str == L"x1") return MouseButtonType::X1;
    else if (str == L"x2") return MouseButtonType::X2;
    return std::nullopt;
}

std::optional<ScriptRunMode> ScriptRunModeFromString(const std::wstring& str)
{
    if (str == L"single" || str == L"單次") return ScriptRunMode::Single;
    else if (str == L"continuous" || str == L"連續")  return ScriptRunMode::Continuous;
    else if (str == L"switch" || str == L"切換") return ScriptRunMode::Switch;
    return std::nullopt;
}

std::wstring ScriptRunModeToString(ScriptRunMode mode) {
#ifdef _LANG_ZH_TW_
    switch (mode) {
    case ScriptRunMode::Single:
        return L"單次";
    case ScriptRunMode::Continuous:
        return L"連續";
    case ScriptRunMode::Switch:
        return L"切換";
    }
#else
    switch (mode) {
    case ScriptRunMode::Single:
        return L"Single";
    case ScriptRunMode::Continuous:
        return L"Continuous";
    case ScriptRunMode::Switch:
        return L"Switch";
    }
#endif // _LANG_ZH_TW_

    return L"";
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

std::wstring HotkeyDataToString(const HotkeyData& data) {
    std::wstring result;

    auto appendModifier = [&result](KeyModifierRequirement requirement, std::wstring_view downName, std::wstring_view upName) {
        if (requirement == KeyModifierRequirement::MustDown) {
            if (!result.empty()) result += L"+";
            result += downName;
        } else if (requirement == KeyModifierRequirement::MustUp) {
            if (!result.empty()) result += L"+";
            result += upName;
        }
    };

    appendModifier(data.ctrl, L"CtrlDown", L"CtrlUp");
    appendModifier(data.shift, L"ShiftDown", L"ShiftUp");
    appendModifier(data.alt, L"AltDown", L"AltUp");

    if (!result.empty()) result += L"+";

    auto keyName = GetNameByKey(data.key);
    result += (keyName.has_value() ? keyName.value() : std::format(L"0x{:X}", data.key));

    return result;
}

bool CheckIsHotkeyDown(const HotkeyData& hotkeyData, const KeyboardEventData& eventData) {
    return eventData.isDown && eventData.vkCode == hotkeyData.key &&
        CheckKeyModifier(eventData.isLCtrlDown, eventData.isRCtrlDown, hotkeyData.ctrl) &&
        CheckKeyModifier(eventData.isLShiftDown, eventData.isRShiftDown, hotkeyData.shift) &&
        CheckKeyModifier(eventData.isLAltDown, eventData.isRAltDown, hotkeyData.alt);
}

bool ParseHotkeyModifier(std::string_view value, HotkeyData& out) {
    if (value == "ctrldown") {
        out.ctrl = KeyModifierRequirement::MustDown;
        return true;
    } else if (value == "ctrlup")
    {
        out.ctrl = KeyModifierRequirement::MustUp;
        return true;
    } else if (value == "shiftdown") {
        out.shift = KeyModifierRequirement::MustDown;
        return true;
    } else if (value == "shiftup") {
        out.shift = KeyModifierRequirement::MustUp;
        return true;
    } else if (value == "altdown") {
        out.alt = KeyModifierRequirement::MustDown;
        return true;
    } else if (value == "altup") {
        out.alt = KeyModifierRequirement::MustUp;
        return true;
    }

    return false;
}

bool ParseHotkeyData(std::wstring_view args, HotkeyData& out) {
    const size_t comma = args.find(L',');

    std::wstring_view keyArg;
    std::wstring_view modifierArgs;

    if (comma == std::wstring_view::npos) {
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
        auto utf8Key = Utf16ToUtf8(std::wstring(keyArg));

        int32_t vkCode = 0;

        if (!StringToInt<int32_t>(utf8Key, vkCode)) return false;

        out.key = vkCode;
    }

    size_t start = 0;
    while (start < modifierArgs.size()) {
        size_t end = modifierArgs.find(L',', start);

        if (end == std::wstring_view::npos) end = modifierArgs.size();

        auto modifier = StringTrim(modifierArgs.substr(start, end - start));

        if (modifier.empty()) return false;

        auto utf8Modifier = Utf16ToUtf8(std::wstring(modifier));

        if (!ParseHotkeyModifier(utf8Modifier, out))return false;

        start = end + 1;
    }

    return true;
}


bool ParseScriptLine(const std::wstring& line, ParsedCommand& outCmd, std::wstring& outError) {
    outError.clear();

    if (line.length() <= 0) {
        return true;
    }

    size_t openParen = line.find('(');
    size_t closeParen = line.rfind(')');
    if (openParen == std::wstring::npos || closeParen == std::wstring::npos || closeParen < openParen) {
#ifdef _LANG_ZH_TW_
        outError = L"缺少或無效的括號 '()'";
#else
        outError = L"Missing or invalid parentheses '()'";
#endif
        return false;
    }

    std::wstring func = std::wstring(StringTrim(StringLower(line.substr(0, openParen))));
    std::wstring args = std::wstring(StringTrim(StringLower(line.substr(openParen + 1, closeParen - openParen - 1))));

    try {
        if (func == L"setstartkey" || func == L"setendkey" || func == L"設定開始按鍵" || func == L"設定停止按鍵") {
            outCmd.type = (func == L"setstartkey" || func == L"設定開始按鍵") ? CmdType::SetStartKey : CmdType::SetEndKey;
            HotkeyData hotkey;
            if (!ParseHotkeyData(args, hotkey)) {
#ifdef _LANG_ZH_TW_
                outError = L"無效的按鍵格式: " + args;
#else
                outError = L"Invalid hotkey format: " + args;
#endif
                return false;
            }
            outCmd.args = hotkey;
            return true;
        }
        else if (func == L"setrunmode" || func == L"設定運行模式") {
            auto runMode = ScriptRunModeFromString(args);
            if (!runMode.has_value()) {
#ifdef _LANG_ZH_TW_
                outError = L"無效的運行模式引數: " + args;
#else
                outError = L"Invalid run mode parameter: " + args;
#endif
                return false;
            }
            outCmd.type = CmdType::SetScriptRunMode;
            outCmd.args = runMode.value();
            return true;
        }
        else if (func == L"delay" || func == L"延遲") {
            outCmd.type = CmdType::Delay;
            int64_t num;
            auto utf8Args = Utf16ToUtf8(args);
            if (!StringToInt<int64_t>(utf8Args, num)) {
#ifdef _LANG_ZH_TW_
                outError = L"無效的延遲引數 (必須為整數): " + args;
#else
                outError = L"Invalid delay parameter (must be an integer): " + args;
#endif
                return false;
            }
            outCmd.args = num;
            return true;
        }
        else if (func == L"mousedown" || func == L"mouseup" || func == L"滑鼠按下" || func == L"滑鼠放開") {
            auto type = MouseButtonTypeFromString(args);
            if (!type.has_value()) {
#ifdef _LANG_ZH_TW_
                outError = L"無效的滑鼠按鍵引數: " + args;
#else
                outError = L"Invalid mouse button parameter: " + args;
#endif
                return false;
            }
            outCmd.type = (func == L"mousedown" || func == L"滑鼠按下") ? CmdType::MouseDown : CmdType::MouseUp;
            outCmd.args = type.value();
            return true;
        }
        else if (func == L"absolutemove" || func == L"relativemove" || func == L"絕對移動" || func == L"相對移動") {
            auto splitView = args | std::views::split(',');

            std::vector<int32_t> splitArgs;

            for (auto&& part : splitView) {
                int32_t num;
                std::string s(part.begin(), part.end());
                s = StringTrim(s);
                if (!StringToInt<int32_t>(s, num)) {
#ifdef _LANG_ZH_TW_
                    outError = L"無效的座標引數 (必須為整數): " + args;
#else
                    outError = L"Invalid coordinate parameter (must be integers): " + args;
#endif
                    return false;
                }
                splitArgs.push_back(num);
            }

            if (splitArgs.size() != 2) {
#ifdef _LANG_ZH_TW_
                outError = L"移動指令只需要2個引數 (x, y): " + args;
#else
                outError = L"Move command expects exactly 2 parameters (x, y): " + args;
#endif
                return false;
            }

            outCmd.type = (func == L"absolutemove" || func == L"絕對移動") ? CmdType::MouseAbsoluteMove : CmdType::MouseRelativeMove;
            outCmd.args = std::move(splitArgs);
            return true;
        }
        else if (func == L"keyboarddown" || func == L"keyboardup" || func == L"鍵盤按下" || func == L"鍵盤放開") {
            auto key = GetKeyByName(args);
            outCmd.type = (func == L"keyboarddown" || func == L"鍵盤按下") ? CmdType::KeyboardDown : CmdType::KeyboardUp;
            if (key.has_value() && key > 0 && key < 65536) {
                outCmd.args = (uint16_t)key.value();
                return true;
            }
            auto utf8Args = Utf16ToUtf8(args);
            uint16_t num = 0;
            if (!StringToInt<uint16_t>(utf8Args, num)) {
#ifdef _LANG_ZH_TW_
                outError = L"無效的按鍵名稱或按鍵編號: " + args;
#else
                outError = L"Invalid key parameter or virtual key code: " + args;
#endif
                return false;
            }
            outCmd.args = (uint16_t)num;
            return true;
        }
        else if (func == L"wheeldown" || func == L"wheelup" || func == L"滾輪下" || func == L"滾輪上") {
            if (!args.empty()) {
#ifdef _LANG_ZH_TW_
                outError = L"滾輪指令不接收引數: " + args;
#else
                outError = L"Wheel command does not accept parameters: " + args;
#endif
                return false;
            }
            outCmd.type = (func == L"wheeldown" || func == L"滾輪下") ? CmdType::WheelDown : CmdType::WheelUp;
            return true;
        }
        else if (func == L"showcoordinate" || func == L"顯示座標") {
            if (!args.empty()) {
#ifdef _LANG_ZH_TW_
                outError = L"顯示座標指令不接收引數: " + args;
#else
                outError = L"ShowCoordinate command does not accept parameters: " + args;
#endif
                return false;
            }
            outCmd.type = CmdType::ShowCoordinate;
            return true;
        } else if (func == L"outputtext" || func == L"輸出文字") {
            size_t firstQuote = args.find_first_of(L"\"'");
            if (firstQuote == std::wstring::npos) {
#ifdef _LANG_ZH_TW_
                outError = L"無效的文字格式（文字應該被\"\"或''包圍）: " + args;
#else
                outError = L"Invalid string format (expected quoted string): " + args;
#endif
                return false;
            }

            wchar_t quote = args[firstQuote];
            size_t secondQuote = args.rfind(quote, firstQuote + 1);
            if (secondQuote == std::wstring::npos) {
#ifdef _LANG_ZH_TW_
                outError = L"無效的文字格式(未閉合的引號): " + args;
#else
                outError = L"Invalid string format (unclosed quote): " + args;
#endif
                return false;
            }

            std::wstring stringPart = args.substr(0, secondQuote + 1);
            std::wstring_view remaining = StringTrim(args.substr(secondQuote + 1));

            std::wstring text;
            if (!ParseStringArg(stringPart, text)) {
#ifdef _LANG_ZH_TW_
                outError = L"無效的字串格式（應為引號字串）: " + args;
#else
                outError = L"Invalid string format (expected quoted string): " + args;
#endif
                return false;
            }

            int64_t delayMs = 0;
            if (!remaining.empty()) {
                if (remaining.front() == L',') {
                    std::wstring delayStr = std::wstring(StringTrim(remaining.substr(1)));
                    auto utf8Delay = Utf16ToUtf8(delayStr);
                    if (!StringToInt<int64_t>(utf8Delay, delayMs)) {
#ifdef _LANG_ZH_TW_
                        outError = L"無效的延遲引數 (必須為整數): " + args;
#else
                        outError = L"Invalid delay parameter (must be an integer): " + args;
#endif
                        return false;
                    }
                }
                else {
#ifdef _LANG_ZH_TW_
                    outError = L"無效的引數格式: " + args;
#else
                    outError = L"Invalid parameter format: " + args;
#endif
                    return false;
                }
            }

            outCmd.type = CmdType::OutputText;
            outCmd.args = OutputTextData{ std::move(text), delayMs };
            return true;
        }
    } catch (...) {
#ifdef _LANG_ZH_TW_
        outError = L"解析該行時發生未知的例外狀況。";
#else
        outError = L"An unexpected exception occurred during line parsing.";
#endif
        return false;
    }

#ifdef _LANG_ZH_TW_
    outError = L"未知的指令: " + func;
#else
    outError = L"Unknown function: " + func;
#endif
    return false;
}

bool ValidateAndParseScript(
    const std::vector<std::wstring>& lines,
    HotkeyData& outStartKey,
    HotkeyData& outEndKey,
    ScriptRunMode& outRunMode,
    std::vector<ParsedCommand>& outParsed, 
    std::wstring& outErrorMsg, 
    int& outErrorLine
) {
    outParsed.clear();
    int lineNumber = 0;

    HotkeyData startKey;
    HotkeyData endKey;
    std::optional<ScriptRunMode> scriptRunMode = std::nullopt;

    for (const auto& rawLine : lines) {
        lineNumber++;
        std::wstring line = std::wstring(StringTrim(rawLine));
        size_t commnentPos = line.find('#');
        if (commnentPos != std::string::npos) {
            line = line.substr(0, commnentPos);
        }

        if (line.empty()) 
            continue;

        ParsedCommand cmd;
        std::wstring outError;
        if (!ParseScriptLine(line, cmd, outError)) {
            outErrorLine = lineNumber;
#ifdef _LANG_ZH_TW_
            outErrorMsg = std::format(L"腳本錯誤\r\n{}\r\n行數: {}:\r\n\"{}\"", outError, lineNumber, rawLine);
#else
            outErrorMsg = std::format(L"Syntax Error\r\n{}\r\nLine {}:\r\n\"{}\"", outError, lineNumber, rawLine);
#endif // _LANG_ZH_TW_
            return false;
        }

        if (cmd.type == CmdType::SetStartKey) {
            if (startKey.key != 0) {
                outErrorLine = lineNumber;
#ifdef _LANG_ZH_TW_
                outErrorMsg = std::format(L"腳本錯誤\r\n已經設定了開始按鍵\r\n行數: {}:\r\n\"{}\"\r\n", lineNumber, rawLine);
#else
                outErrorMsg = std::format(L"Syntax Error\r\nStart Key already defined\r\nLine {}:\r\n\"{}\"", lineNumber, rawLine);
#endif // _LANG_ZH_TW_
                return false;
            }
            startKey = std::any_cast<HotkeyData>(cmd.args);
        } else if (cmd.type == CmdType::SetEndKey) {
            if (endKey.key != 0) {
                outErrorLine = lineNumber;
#ifdef _LANG_ZH_TW_
                outErrorMsg = std::format(L"腳本錯誤，行數: {}:\r\n\"{}\"\r\n已經設定了停止按鍵", lineNumber, rawLine);
#else
                outErrorMsg = std::format(L"Syntax Error\r\nEnd Key already defined\r\nLine {}:\r\n\"{}\"", lineNumber, rawLine);
#endif // _LANG_ZH_TW_
                return false;
            }
            endKey = std::any_cast<HotkeyData>(cmd.args);
        } else if (cmd.type == CmdType::SetScriptRunMode) {
            if (scriptRunMode.has_value()) {
                outErrorLine = lineNumber;
#ifdef _LANG_ZH_TW_
                outErrorMsg = std::format(L"腳本錯誤，行數: {}:\r\n\"{}\"\r\n已經設定了運行模式", lineNumber, rawLine);
#else
                outErrorMsg = std::format(L"Syntax Error\r\nRun Mode already defined\r\nLine {}:\r\n\"{}\"", lineNumber, rawLine);
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
    outRunMode = scriptRunMode.has_value() ? scriptRunMode.value() : ScriptRunMode::Single;
    return true;
}

void ExecuteParsedCommand(std::stop_token stopToken, const ParsedCommand& cmd) {
    switch (cmd.type) {
    case CmdType::Delay:
        InputSim::Delay(stopToken, std::any_cast<int64_t>(cmd.args));
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
    case CmdType::ShowCoordinate:
        InputSim::ShowCoordinate();
        break;
    case CmdType::OutputText: {
        const auto& data = std::any_cast<const OutputTextData&>(cmd.args);
        InputSim::OutputText(stopToken, data.text, data.delayMs);
        break;
    }
    }
}
void ExecuteScriptWorker(std::stop_token stopToken, ScriptData& data) {
    data.isRunning = true;
    if (data.preExecute != nullptr) data.preExecute();
    do {
        for (const auto& cmd : data.commands) {
            if (stopToken.stop_requested() || data.isStopRequested) break;
            ExecuteParsedCommand(stopToken, cmd);
        }

        if (data.mode == ScriptRunMode::Single) break;

    } while (!stopToken.stop_requested() && !data.isStopRequested);

    data.isRunning = false;
    if (data.postExecute != nullptr) data.postExecute(false);
}

bool ReadUtf8FileLines(const std::wstring& filePath, std::vector<std::wstring>& outLines) {
    outLines.clear();
    std::ifstream file(filePath, std::ios::in | std::ios::binary);
    if (!file.is_open()) return false;

    std::string content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    file.close();

    // Skip UTF-8 BOM
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

        outLines.push_back(Utf8ToUtf16(line));
    }
    return true;
}

bool StartScriptExecution(ScriptData& data) {
    if (data.isRunning) {
        return false;
    }
    data.isStopRequested = false;
    data.executeThread = std::jthread(ExecuteScriptWorker, std::ref(data));
    return true;
}

bool StopScriptExecution(ScriptData& data, bool waitScriptEnd) {
    if (!data.isRunning) {
        return false;
    }

    data.isStopRequested = true;
    if (data.executeThread.joinable()) {
        data.executeThread.request_stop();
        if (waitScriptEnd) data.executeThread.join();
    }
    return true;
}

bool CListCtrlPreTranslateMessageClipboard(MSG* pMsg, const CDialog* dlg, const CListCtrl& listCtrl) {
    if (pMsg->message == WM_KEYDOWN && (dlg->GetFocus()->GetSafeHwnd() == listCtrl.GetSafeHwnd())) {
        if (pMsg->wParam == 'C' && (GetKeyState(VK_CONTROL) & 0x8000)) {

            CString clipboardData = _T("");
            int nColumnCount = listCtrl.GetHeaderCtrl()->GetItemCount();

            POSITION pos = listCtrl.GetFirstSelectedItemPosition();
            while (pos != NULL) {
                int nItem = listCtrl.GetNextSelectedItem(pos);
                CString rowText = _T("");

                for (int nCol = 0; nCol < nColumnCount; ++nCol) {
                    CString cellText = listCtrl.GetItemText(nItem, nCol);
                    rowText += cellText;

                    if (nCol < nColumnCount - 1) {
                        rowText += _T("\t");
                    }
                }

                clipboardData += rowText + _T("\r\n");
            }

            if (!clipboardData.IsEmpty() && OpenClipboard(dlg->GetSafeHwnd())) {
                HGLOBAL hGlobal = GlobalAlloc(GMEM_MOVEABLE, (clipboardData.GetLength() + 1) * sizeof(TCHAR));
                if (hGlobal != nullptr) {
                    TCHAR* pData = (TCHAR*)GlobalLock(hGlobal);
                    if (pData != nullptr) {
                        _tcscpy_s(pData, clipboardData.GetLength() + 1, clipboardData);
                        GlobalUnlock(hGlobal);
                        EmptyClipboard();
                        SetClipboardData(CF_UNICODETEXT, hGlobal);
                    }
                }
                CloseClipboard();
            }

            return true;
        }
    }
    return false;
}

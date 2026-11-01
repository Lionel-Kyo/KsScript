#pragma once
#ifndef _I18N_H_
#define _I18N_H_

#include <string>
#include <vector>
#include <cstdint>
#include <format>

enum class Language : int32_t {
    English = 0,
    Cantonese,
    TraditionalChinese,
    SimplifiedChinese,
    Japanese,
    Korean,
    Spanish,
    Portuguese,
    French,
    German,
    Count
};

enum class StringId : size_t {
    AppTitle,
    MainWndTitle,
    OpenScripts,
    ScriptRecorder,
    FunctionsRef,
    KeysRef,
    ColEnable,
    ColScriptName,
    ColExecutionMode,
    ColStartKey,
    ColEndKey,
    MenuDisable,
    MenuEnable,
    MenuStopTestRun,
    MenuRunTest,
    MenuRemove,
    MenuReload,
    MenuOpenScriptLocation,
    MenuChangeKeysAndMode,
    EnableAll,
    DisableAll,
    RemoveAll,
    Save,
    Cancel,
    Ok,
    StartKeyLabel,
    EndKeyLabel,
    RunModeLabel,
    Record,
    StopRec,
    Play,
    StopPlay,
    SaveScript,
    StatusRecording,
    StatusPlaying,
    StatusIdle,
    ColName,
    ColParameters,
    ColExplanation,
    ColValue,
    TitleFileError,
    TitleScriptError,
    TitleSaveFailed,
    TitleSaveSuccess,
    MsgReadScriptFailed,
    MsgReadFileFailed,
    MsgOpenFileWriteFailed,
    MsgWriteFileUnexpected,
    MsgScriptSettingsUpdated,
    MsgScriptSavedTo,
    MsgCurrentCoordinate,
    // Parser errors
    ErrMissingParentheses,
    ErrTrailingAfterParen,
    ErrInvalidHotkey,
    ErrInvalidRunMode,
    ErrInvalidDelay,
    ErrInvalidMouseButton,
    ErrInvalidCoordinate,
    ErrMoveArgCount,
    ErrInvalidKeyParam,
    ErrWheelNoParam,
    ErrInvalidStringQuotes,
    ErrUnclosedQuote,
    ErrInvalidParamFormat,
    ErrShowCoordNoParam,
    ErrParseException,
    ErrUnknownFunction,
    ErrSyntaxErrorPrefix,
    ErrStartKeyAlreadyDefined,
    ErrEndKeyAlreadyDefined,
    ErrRunModeAlreadyDefined,
    // Execution modes
    ModeFullSingle,
    ModeFullContinuous,
    ModeFullSwitch,
    ModeSingle,
    ModeContinuous,
    ModeSwitch,
    Count
};

struct FuncInfo {
    std::u8string name;
    std::u8string parameters;
    std::u8string explanation;
};

class I18n {
public:
    static void Init();
    static Language GetLanguage();
    static void SetLanguage(Language lang);
    static const char* Get(StringId id);
    static std::u8string GetU8(StringId id);
    static std::wstring GetW(StringId id);
    static const char* const* GetLanguageNames();
    static const std::vector<FuncInfo>& GetFuncInfos();
    static const char* GetRunModeString(int modeIndex);
    static void LoadConfig();
    static void SaveConfig();

    static std::string WideToUtf8(std::wstring_view value);
    static std::u8string WideToUtf8U8(std::wstring_view value);
    static std::wstring Utf8ToWide(std::string_view value);

    template<typename... Args>
    static std::u8string FormatU8(StringId id, Args&&... args) {
        const std::u8string format = GetU8(id);
        const std::string_view formatView(reinterpret_cast<const char*>(format.data()), format.size());

        auto converted = std::tuple{ ToFormatArg(std::forward<Args>(args))... };
        const std::string result = std::apply(
            [&](auto&... values) {
                return std::vformat(formatView, std::make_format_args(values...));
            },
            converted
        );

        return std::u8string(reinterpret_cast<const char8_t*>(result.data()), result.size());
    }

    template<typename... Args>
    static std::wstring FormatW(StringId id, Args&&... args) {
        return std::vformat(GetW(id), std::make_wformat_args(args...));
    }
private:
    template<typename T>
    static auto ToFormatArg(T&& value) {
        using U = std::remove_cvref_t<T>;

        if constexpr (std::is_same_v<U, std::u8string> ||
            std::is_same_v<U, std::u8string_view>) {
            return std::string(
                reinterpret_cast<const char*>(value.data()),
                value.size());
        }
        else if constexpr (std::is_same_v<U, std::wstring> ||
            std::is_same_v<U, std::wstring_view>) {
            return WideToUtf8(value);
        }
        else if constexpr (std::is_same_v<U, const char8_t*> ||
            std::is_same_v<U, char8_t*>) {
            return std::string(
                reinterpret_cast<const char*>(value));
        }
        else if constexpr (std::is_same_v<U, const wchar_t*> ||
            std::is_same_v<U, wchar_t*>) {
            return WideToUtf8(value);
        }
        else {
            return std::forward<T>(value);
        }
    }
};

#endif // !_I18N_H_

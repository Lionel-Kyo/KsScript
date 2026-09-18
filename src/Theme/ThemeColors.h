#pragma once
#include <afxwin.h>
#include <uxtheme.h>

#ifndef DWMWA_USE_IMMERSIVE_DARK_MODE
#define DWMWA_USE_IMMERSIVE_DARK_MODE 20
#endif

namespace DarkTheme {
    enum class PreferredAppMode { Default, AllowDark, ForceDark, ForceLight, Max };
    using fnSetPreferredAppMode = PreferredAppMode(WINAPI*)(PreferredAppMode);
    using fnAllowDarkModeForWindow = BOOL(WINAPI*)(HWND, BOOL);

    void EnableControlDarkMode(HWND hWnd);
    void EnableDarkTitleBar(HWND hwnd);

    constexpr COLORREF ContainerBackColor = RGB(28, 28, 28);
    constexpr COLORREF ComponentSubtleBackColor = RGB(37, 37, 38);
    constexpr COLORREF ComponentBackColor = RGB(45, 45, 48);

    constexpr COLORREF ForeColor = RGB(241, 241, 241);
    constexpr COLORREF SubtleBorderColor = RGB(63, 63, 70);
    constexpr COLORREF BorderColor = RGB(85, 85, 85);
    constexpr COLORREF SeparatorColor = RGB(51, 51, 51);

    constexpr COLORREF HoverColor = RGB(65, 55, 85);
    constexpr COLORREF FocusColor = RGB(40, 35, 55);
    constexpr COLORREF PrimaryFocusColor = RGB(190, 160, 255);
    constexpr COLORREF PressColor = RGB(100, 80, 140);

    constexpr COLORREF ButtonCheckedBackColor = RGB(180, 160, 220);
    constexpr COLORREF ButtonCheckedForeColor = RGB(30, 30, 30);
    constexpr COLORREF ButtonCheckedFocusColor = RGB(200, 185, 235);
    constexpr COLORREF ButtonCheckedHoverColor = RGB(220, 210, 250);
    constexpr COLORREF ButtonCheckedPressColor = RGB(235, 225, 255);
}
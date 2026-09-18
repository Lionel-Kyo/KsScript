#include "pch.h"
#include "ThemeColors.h"

#pragma comment(lib, "dwmapi.lib")

void DarkTheme::EnableControlDarkMode(HWND hWnd) {
    HMODULE hUxTheme = LoadLibraryExW(_T("uxtheme.dll"), NULL, LOAD_LIBRARY_SEARCH_SYSTEM32);
    if (hUxTheme) {
        auto SetPreferredAppMode = (fnSetPreferredAppMode)GetProcAddress(hUxTheme, MAKEINTRESOURCEA(135));
        auto AllowDarkModeForWindow = (fnAllowDarkModeForWindow)GetProcAddress(hUxTheme, MAKEINTRESOURCEA(133));

        if (SetPreferredAppMode) SetPreferredAppMode(PreferredAppMode::ForceDark);
        if (AllowDarkModeForWindow) AllowDarkModeForWindow(hWnd, TRUE);

        FreeLibrary(hUxTheme);
    }
    SetWindowTheme(hWnd, L"DarkMode_Explorer", NULL);
    //SetWindowTheme(hWnd, L"Explorer", NULL);
}

void DarkTheme::EnableDarkTitleBar(HWND hwnd) {
    BOOL useDarkMode = TRUE;
    DwmSetWindowAttribute(hwnd, DWMWA_USE_IMMERSIVE_DARK_MODE, &useDarkMode, sizeof(useDarkMode));
}
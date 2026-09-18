#pragma once
#define _AFXDLL
#include <afxwin.h>
#include <Windows.h>
#include <shellapi.h>
#include <tlhelp32.h>
#include <psapi.h>
#include <mmsystem.h>

class CMainApp : public CWinApp {
public:
    CMainApp() = default;
    virtual BOOL InitInstance() override;
private:
    static bool IsRunAsAdmin();
    static bool RestartAsAdmin();
    static HWND FindMainWindow(DWORD processId);
    static bool ShowAndFocusWindow();
};

extern CMainApp theApp;
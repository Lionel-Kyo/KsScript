#pragma once
#define _AFXDLL
#include <afxwin.h>

class CMainApp : public CWinApp {
public:
    CMainApp() = default;
    virtual BOOL InitInstance() override;
};

extern CMainApp theApp;
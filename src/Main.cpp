#include "pch.h"
#include "Main.h"
#include "MainDlg.h"
#include <timeapi.h>

CMainApp theApp;

BOOL CMainApp::InitInstance() {
    CWinApp::InitInstance();
    timeBeginPeriod(1);

    CMainDlg dlg;
    m_pMainWnd = &dlg;
    dlg.DoModal();

    timeEndPeriod(1);
    return FALSE;
}
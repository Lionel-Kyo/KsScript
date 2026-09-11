#include "pch.h"
#include "MainDlg.h"

CMainDlg::CMainDlg(CWnd* pParent) : CDialog(IDD_MAIN_DIALOG, pParent) {}

void CMainDlg::DoDataExchange(CDataExchange* pDX) {
    CDialog::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_BTN_LOAD_SCRIPTS, m_btnLoadScripts);
    DDX_Control(pDX, IDC_BTN_OPEN_RECORD, m_btnOpenRecordDlg);
    DDX_Control(pDX, IDC_BTN_OPEN_FUNCTIONS_REF, m_btnOpenFunctionsRefDlg);
    DDX_Control(pDX, IDC_BTN_OPEN_KEYS_REF, m_btnOpenKeysRefDlg);
    DDX_Control(pDX, IDC_LST_SCRIPTS, m_lstScripts);
}

BEGIN_MESSAGE_MAP(CMainDlg, CDialog)
    ON_WM_SIZE()
    ON_WM_CLOSE()
    ON_BN_CLICKED(IDC_BTN_LOAD_SCRIPTS, &CMainDlg::OnBtnLoadScripts)
    ON_BN_CLICKED(IDC_BTN_OPEN_RECORD, &CMainDlg::OnBtnOpenRecordDlg)
    ON_BN_CLICKED(IDC_BTN_OPEN_FUNCTIONS_REF, &CMainDlg::OnBtnOpenFunctionsRefDlg)
    ON_BN_CLICKED(IDC_BTN_OPEN_KEYS_REF, &CMainDlg::OnBtnOpenKeysRefDlg)
    ON_NOTIFY(NM_CUSTOMDRAW, IDC_LST_SCRIPTS, &CMainDlg::OnNMCustomdrawLstScripts)
    ON_NOTIFY(HDN_ITEMCHANGED, 0, &CMainDlg::OnHeaderItemChanged)
    ON_NOTIFY(NM_CLICK, IDC_LST_SCRIPTS, &CMainDlg::OnNMClickLstScripts)
    ON_NOTIFY(NM_RCLICK, IDC_LST_SCRIPTS, &CMainDlg::OnNMRClickLstScripts)
    ON_NOTIFY(LVN_ITEMCHANGED, IDC_LST_SCRIPTS, &CMainDlg::OnLvnItemchangedLstScripts)
END_MESSAGE_MAP()

BOOL CMainDlg::OnInitDialog() {
    CDialog::OnInitDialog();

    HICON hIcon = AfxGetApp()->LoadIcon(IDI_APP_ICON);
    if (hIcon != nullptr) {
        SetIcon(hIcon, TRUE);
        SetIcon(hIcon, FALSE);
    }

#ifdef _LANG_ZH_TW_
    m_btnLoadScripts.SetWindowTextW(_T("讀取腳本"));
    m_btnOpenRecordDlg.SetWindowTextW(_T("腳本記錄器"));
    m_btnOpenFunctionsRefDlg.SetWindowTextW(_T("腳本指令說明"));
    m_btnOpenKeysRefDlg.SetWindowTextW(_T("鍵盤按鍵說明"));
#else
    m_btnLoadScripts.SetWindowTextW(_T("Open Scripts"));
    m_btnOpenRecordDlg.SetWindowTextW(_T("Script Recorder"));
    m_btnOpenFunctionsRefDlg.SetWindowTextW(_T("Functions Reference"));
    m_btnOpenKeysRefDlg.SetWindowTextW(_T("Keys Reference"));
#endif // _LANG_ZH_TW_

    MainHelper::GetInstance().SetHWnd(m_hWnd);
    MainHelper::GetInstance().StartKeyboardHook();

    m_lstScripts.SetExtendedStyle(
        LVS_EX_FULLROWSELECT |
        LVS_EX_GRIDLINES //|
        //LVS_EX_CHECKBOXES
    );
#ifdef _LANG_ZH_TW_
    m_lstScripts.InsertColumn(0, _T("開啟"), LVCFMT_CENTER, 60);
    m_lstScripts.InsertColumn(1, _T("腳本名稱"), LVCFMT_LEFT, 240);
    m_lstScripts.InsertColumn(2, _T("運行模式"), LVCFMT_LEFT, 120);
    m_lstScripts.InsertColumn(3, _T("開始按鍵"), LVCFMT_LEFT, 120);
    m_lstScripts.InsertColumn(4, _T("停止按鍵"), LVCFMT_LEFT, 120);
#else
    m_lstScripts.InsertColumn(0, _T("Enable"), LVCFMT_CENTER, 60);
    m_lstScripts.InsertColumn(1, _T("Script Name"), LVCFMT_LEFT, 240);
    m_lstScripts.InsertColumn(2, _T("Execution Mode"), LVCFMT_LEFT, 120);
    m_lstScripts.InsertColumn(3, _T("Start Key"), LVCFMT_LEFT, 120);
    m_lstScripts.InsertColumn(4, _T("End Key"), LVCFMT_LEFT, 120);
#endif // _LANG_ZH_TW_

    CRect rect;
    GetClientRect(&rect);
    ResizeDlg(rect.Width(), rect.Height());

    return TRUE;
}

BOOL CMainDlg::PreTranslateMessage(MSG* pMsg) {
    if (pMsg->message == WM_KEYDOWN && pMsg->wParam == VK_ESCAPE) {
        return TRUE;
    } else if (CListCtrlPreTranslateMessageClipboard(pMsg, this, m_lstScripts)) {
        return TRUE;
    }

    return CDialog::PreTranslateMessage(pMsg);
}

bool CMainDlg::LoadScriptEntry(const std::wstring& filePath, std::wstring& outErrorContent, std::wstring& outErrorTitle, ScriptEntry& outEntry) {
    ScriptEntry entry;
    entry.filePath = filePath;
    entry.fileName = std::filesystem::path(filePath).filename().wstring();;

    std::vector<std::wstring> fileLines;
    if (!ReadUtf8FileLines(entry.filePath, fileLines)) {
#ifdef _LANG_ZH_TW_
        outErrorContent = std::format(L"讀取UTF-8 .txt腳本文件失敗\r\n{}", entry.filePath);
        outErrorTitle = L"文件錯誤";
#else
        outErrorContent = std::format(L"Failed to read UTF-8 script file.\r\n{}", entry.filePath);
        outErrorTitle = L"File Error";
#endif // _LANG_ZH_TW_
        return false;
    }
    entry.lines = fileLines;

    std::vector<ParsedCommand> parsedCommands;
    HotkeyData startKey;
    HotkeyData endKey;
    ScriptRunMode runMode;
    std::wstring errorMsg;
    int errorLine = 0;
    if (!ValidateAndParseScript(fileLines, startKey, endKey, runMode, parsedCommands, errorMsg, errorLine)) {
        outErrorContent = errorMsg;
#ifdef _LANG_ZH_TW_
        outErrorTitle = L"腳本錯誤";
#else
        outErrorTitle = L"Script Syntax Error";
#endif // _LANG_ZH_TW_
        return false;
    }
    switch (runMode) {
    case ScriptRunMode::Single:
        endKey = HotkeyData();
        break;
    case ScriptRunMode::Continuous:
        endKey = startKey;
        break;
    }
    entry.data.startKey = startKey;
    entry.data.endKey = endKey;
    entry.data.mode = runMode;
    entry.data.commands = parsedCommands;

    outEntry = std::move(entry);
    return true;
}

void CMainDlg::OnBtnOpenRecordDlg() {
    if (m_recordDlg == nullptr) {
        m_recordDlg = new CRecordDlg(CWnd::GetDesktopWindow(), [&]() { m_recordDlg = nullptr; });

        if (m_recordDlg->Create(CRecordDlg::IDD, CWnd::GetDesktopWindow())) {
            m_recordDlg->ShowWindow(SW_SHOW);
        }
    } else {
        m_recordDlg->SetForegroundWindow();
    }
}

void CMainDlg::OnBtnOpenFunctionsRefDlg() {
    if (m_functionsRefDlg == nullptr) {
        m_functionsRefDlg = new CFunctionsRefDlg(CWnd::GetDesktopWindow(), [&]() { m_functionsRefDlg = nullptr; });

        if (m_functionsRefDlg->Create(CFunctionsRefDlg::IDD, CWnd::GetDesktopWindow())) {
            m_functionsRefDlg->ShowWindow(SW_SHOW);
        }
    } else {
        m_functionsRefDlg->SetForegroundWindow();
    }
}

void CMainDlg::OnBtnOpenKeysRefDlg() {
    if (m_keysRefDlg == nullptr) {
        m_keysRefDlg = new CKeysRefDlg(CWnd::GetDesktopWindow(), [&]() { m_keysRefDlg = nullptr; });

        if (m_keysRefDlg->Create(CKeysRefDlg::IDD, CWnd::GetDesktopWindow())) {
            m_keysRefDlg->ShowWindow(SW_SHOW);
        }
    }
    else {
        m_keysRefDlg->SetForegroundWindow();
    }
}

void CMainDlg::OnBtnLoadScripts() {
    // 64KB buffer to support selecting dozens of files at once
    const DWORD maxFiles = 200;
    const DWORD buffSize = maxFiles * (MAX_PATH + 1) + 1;
    std::vector<wchar_t> buffer(buffSize, 0);

    CFileDialog fileDlg(TRUE, _T("txt"), nullptr,
        OFN_ALLOWMULTISELECT | OFN_FILEMUSTEXIST | OFN_HIDEREADONLY,
        _T("UTF-8 Text Scripts (*.txt)|*.txt|All Files (*.*)|*.*||"), this);

    fileDlg.m_ofn.lpstrFile = buffer.data();
    fileDlg.m_ofn.nMaxFile = buffSize;

    if (fileDlg.DoModal() == IDOK) {
        POSITION pos = fileDlg.GetStartPosition();
        std::vector<ScriptEntry> loadedScripts;

        bool hasError = false;
        while (pos != nullptr) {
            ScriptEntry entry;
            std::wstring errorContent;
            std::wstring errorTitle;
            if (!LoadScriptEntry(fileDlg.GetNextPathName(pos).GetString(), errorContent, errorTitle, entry)) {
                ::MessageBoxW(m_hWnd, errorContent.c_str(), errorTitle.c_str(), MB_ICONERROR | MB_OK);
                hasError = true;
                break;
            }
            loadedScripts.push_back(std::move(entry));
        }

        if (hasError) {
            return;
        }
        for (auto& _entry : loadedScripts) {
            {
                std::lock_guard <std::mutex> lock(MainHelper::GetInstance().LoadedScriptsMutex());
                MainHelper::GetInstance().LoadedScripts().push_back(std::move(_entry));
            }
            auto& entry = MainHelper::GetInstance().LoadedScripts().back();
            int idx = m_lstScripts.GetItemCount();
            m_lstScripts.InsertItem(idx, _T(""));
            m_lstScripts.SetItemText(idx, 1, entry.fileName.c_str());
            m_lstScripts.SetItemText(idx, 2, ScriptRunModeToString(entry.data.mode).c_str());
            m_lstScripts.SetItemText(idx, 3, HotkeyDataToString(entry.data.startKey).c_str());
            m_lstScripts.SetItemText(idx, 4, HotkeyDataToString(entry.data.endKey).c_str());
            m_lstScripts.SetCheck(idx, FALSE);
        }
    }
}

void CMainDlg::ResizeDlg(int cx, int cy) {
    if (m_lstScripts.GetSafeHwnd()) {
        m_lstScripts.MoveWindow(10, 60, cx - 20, cy - 75);
    }
    if (::IsWindow(m_lstScripts.GetSafeHwnd())) {
        ResizeScriptListColumns();
    }
}

void CMainDlg::ResizeScriptListColumns() {
    CRect rc;
    m_lstScripts.GetClientRect(&rc);

    int fixedWidth = 0;

    for (int i = 0; i < 4; ++i)
        fixedWidth += m_lstScripts.GetColumnWidth(i);

    int width = rc.Width() - fixedWidth;

    if (width < 120) width = 120;

    m_lstScripts.SetColumnWidth(4, width);
}

void CMainDlg::OnNMCustomdrawLstScripts(NMHDR* pNMHDR, LRESULT* pResult) {
    auto* pLVCD = reinterpret_cast<NMLVCUSTOMDRAW*>(pNMHDR);

    *pResult = CDRF_DODEFAULT;

    if (pLVCD->nmcd.dwDrawStage == CDDS_PREPAINT) {
        *pResult = CDRF_NOTIFYITEMDRAW;
        return;
    }

    if (pLVCD->nmcd.dwDrawStage == CDDS_ITEMPREPAINT) {
        *pResult = CDRF_NOTIFYSUBITEMDRAW;
        return;
    }

    if (pLVCD->nmcd.dwDrawStage == (CDDS_ITEMPREPAINT | CDDS_SUBITEM)) {
        const int row = static_cast<int>(pLVCD->nmcd.dwItemSpec);
        const int column = pLVCD->iSubItem;

        if (column != 0)
        {
            *pResult = CDRF_DODEFAULT;
            return;
        }

        CRect cellRect;
        m_lstScripts.GetSubItemRect(row, column, LVIR_BOUNDS, cellRect);

        cellRect.right = cellRect.left + m_lstScripts.GetColumnWidth(0);

        constexpr int checkSize = 16;

        const int x = cellRect.left + (cellRect.Width() - checkSize) / 2;
        const int y = cellRect.top + (cellRect.Height() - checkSize) / 2;

        CRect checkRect(x, y, x + checkSize, y + checkSize);

        const bool checked = m_lstScripts.GetCheck(row) != FALSE;
        CDC* dc = CDC::FromHandle(pLVCD->nmcd.hdc);

        HTHEME hTheme = OpenThemeData(m_lstScripts.m_hWnd, L"Button");
        if (hTheme) {
            int stateId = checked ? CBS_CHECKEDNORMAL : CBS_UNCHECKEDNORMAL;
            DrawThemeBackground(hTheme, dc->GetSafeHdc(), BP_CHECKBOX, stateId, &checkRect, nullptr);
            CloseThemeData(hTheme);
        } else {
            UINT state = DFCS_BUTTONCHECK;
            if (checked) state |= DFCS_CHECKED;
            dc->DrawFrameControl(&checkRect, DFC_BUTTON, state);
        }

        *pResult = CDRF_SKIPDEFAULT;
        return;
    }
}

void CMainDlg::OnHeaderItemChanged(NMHDR* pNMHDR, LRESULT* pResult) {
    LPNMHEADER phdr = reinterpret_cast<LPNMHEADER>(pNMHDR);
    m_lstScripts.Invalidate();
    *pResult = 0;
}

void CMainDlg::OnNMClickLstScripts(NMHDR* pNMHDR, LRESULT* pResult) {
    auto* pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);

    if (pNMItemActivate->iItem < 0 || pNMItemActivate->iSubItem != 0) {
        *pResult = 0;
        return;
    }

    const int row = pNMItemActivate->iItem;

    CRect cellRect;
    m_lstScripts.GetSubItemRect(row, 0, LVIR_BOUNDS, cellRect);

    cellRect.right = cellRect.left + m_lstScripts.GetColumnWidth(0);

    if (cellRect.PtInRect(pNMItemActivate->ptAction)) {
        const bool checked = m_lstScripts.GetCheck(row) != FALSE;
        m_lstScripts.SetCheck(row, checked ? 0 : 1);
        m_lstScripts.RedrawItems(row, row);
        return;
    }

    *pResult = 0;
}

void CMainDlg::OnNMRClickLstScripts(NMHDR* pNMHDR, LRESULT* pResult) {
    LPNMITEMACTIVATE pItem = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);

    if (pItem->iItem < 0) {
        *pResult = 0;
        return;
    }

    int row = pItem->iItem;

    m_lstScripts.SetItemState(row, LVIS_SELECTED, LVIS_SELECTED);

    CMenu menu;
    menu.CreatePopupMenu();

    bool isRowEnabled = m_lstScripts.GetCheck(row) != FALSE;
#ifdef _LANG_ZH_TW_
    menu.AppendMenu(MF_STRING, ID_SCRIPT_ENABLE_ALL, _T("開啟全部腳本"));
    menu.AppendMenu(MF_STRING, ID_SCRIPT_DISABLE_ALL, _T("關閉全部腳本"));
    menu.AppendMenu(MF_STRING, ID_SCRIPT_REMOVE_ALL, _T("移除全部腳本"));
    menu.AppendMenu(MF_SEPARATOR);
    if (isRowEnabled) {
        menu.AppendMenu(MF_STRING, ID_SCRIPT_OFF, _T("關閉腳本"));
    }
    else {
        menu.AppendMenu(MF_STRING, ID_SCRIPT_ON, _T("開啟腳本"));
    }
    menu.AppendMenu(MF_SEPARATOR);
    menu.AppendMenu(MF_STRING, ID_SCRIPT_REMOVE, _T("移除腳本"));
    menu.AppendMenu(MF_STRING, ID_SCRIPT_RELOAD, _T("重新讀取"));
#else
    menu.AppendMenu(MF_STRING, ID_SCRIPT_ENABLE_ALL, _T("Enable All"));
    menu.AppendMenu(MF_STRING, ID_SCRIPT_DISABLE_ALL, _T("Disable All"));
    menu.AppendMenu(MF_STRING, ID_SCRIPT_REMOVE_ALL, _T("Remove All"));
    menu.AppendMenu(MF_SEPARATOR);
    if (isRowEnabled) {
        menu.AppendMenu(MF_STRING, ID_SCRIPT_OFF, _T("Off"));
    }
    else {
        menu.AppendMenu(MF_STRING, ID_SCRIPT_ON, _T("On"));
    }
    menu.AppendMenu(MF_SEPARATOR);
    menu.AppendMenu(MF_STRING, ID_SCRIPT_REMOVE, _T("Remove"));
    menu.AppendMenu(MF_STRING, ID_SCRIPT_RELOAD, _T("Reload"));
#endif // _LANG_ZH_TW_

    CPoint pt;
    GetCursorPos(&pt);
    UINT command = menu.TrackPopupMenu(TPM_LEFTALIGN | TPM_TOPALIGN | TPM_RIGHTBUTTON | TPM_RETURNCMD, pt.x, pt.y, this);

    switch (command)
    {

    case ID_SCRIPT_ENABLE_ALL:
    {
        int itemCount = m_lstScripts.GetItemCount();

        for (int i = 0; i < itemCount; ++i) {
            m_lstScripts.SetCheck(i, TRUE);
        }
    }
    break;

    case ID_SCRIPT_DISABLE_ALL:
    {
        int itemCount = m_lstScripts.GetItemCount();

        for (int i = 0; i < itemCount; ++i) {
            m_lstScripts.SetCheck(i, FALSE);
        }
    }
    break;

    case ID_SCRIPT_REMOVE_ALL:
    {
        std::lock_guard<std::mutex> lock(MainHelper::GetInstance().LoadedScriptsMutex());
        m_lstScripts.DeleteAllItems();
        auto& entries = MainHelper::GetInstance().LoadedScripts();
        for (auto& entry : entries) {
            StopScriptExecution(entry.data, false);
        }
        entries.clear();
    }
    break;

    case ID_SCRIPT_ON:
        m_lstScripts.SetCheck(row, TRUE);
        break;

    case ID_SCRIPT_OFF:
        m_lstScripts.SetCheck(row, FALSE);
        break;

    case ID_SCRIPT_REMOVE:
        {
            std::lock_guard<std::mutex> lock(MainHelper::GetInstance().LoadedScriptsMutex());
            m_lstScripts.DeleteItem(row);
            auto& entry = MainHelper::GetInstance().LoadedScripts().at(row);
            StopScriptExecution(entry.data, false);
            MainHelper::GetInstance().LoadedScripts().erase(MainHelper::GetInstance().LoadedScripts().begin() + row);
        }
        break;

    case ID_SCRIPT_RELOAD:
        {
            std::lock_guard<std::mutex> lock(MainHelper::GetInstance().LoadedScriptsMutex());
            auto& oldEntry = MainHelper::GetInstance().LoadedScripts().at(row);
            StopScriptExecution(oldEntry.data, false);
            ScriptEntry entry;
            std::wstring errorContent;
            std::wstring errorTitle;
            if (!LoadScriptEntry(oldEntry.filePath, errorContent, errorTitle, entry)) {
                ::MessageBoxW(m_hWnd, errorContent.c_str(), errorTitle.c_str(), MB_ICONERROR | MB_OK);
            } else {
                m_lstScripts.SetItemText(row, 2, ScriptRunModeToString(entry.data.mode).c_str());
                m_lstScripts.SetItemText(row, 3, HotkeyDataToString(entry.data.startKey).c_str());
                m_lstScripts.SetItemText(row, 4, HotkeyDataToString(entry.data.endKey).c_str());
                MainHelper::GetInstance().LoadedScripts()[row] = std::move(entry);
            }

        }
        break;
    }

    *pResult = 0;
}

void CMainDlg::OnLvnItemchangedLstScripts(NMHDR* pNMHDR, LRESULT* pResult) {
    LPNMLISTVIEW pNMLV = reinterpret_cast<LPNMLISTVIEW>(pNMHDR);

    if (pNMLV->iItem >= 0 && (pNMLV->uChanged & LVIF_STATE))
    {
        UINT oldState = pNMLV->uOldState & LVIS_STATEIMAGEMASK;
        UINT newState = pNMLV->uNewState & LVIS_STATEIMAGEMASK;

        if (oldState != newState)
        {
            bool enabled = m_lstScripts.GetCheck(pNMLV->iItem) != FALSE;
            {
                std::lock_guard<std::mutex> lock(MainHelper::GetInstance().LoadedScriptsMutex());
                MainHelper::GetInstance().LoadedScripts().at(pNMLV->iItem).data.isEnabled = enabled;
            }
        }
    }

    *pResult = 0;
}

void CMainDlg::OnSize(UINT nType, int cx, int cy) {
    CDialog::OnSize(nType, cx, cy);
    ResizeDlg(cx, cy);
}

void CMainDlg::OnClose() {
    MainHelper::GetInstance().StopKeyboardHook();
    CDialog::OnClose();
}


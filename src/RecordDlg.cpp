#include "pch.h"
#include "RecordDlg.h"

CRecordDlg::CRecordDlg(CWnd* pParent, std::function<void()> onDestory) : CDialog(IDD_RECORD_DIALOG, pParent) {
    m_onDestory = onDestory;
}

void CRecordDlg::DoDataExchange(CDataExchange* pDX) {
    CDialog::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_BTN_RECORD, m_btnRecord);
    DDX_Control(pDX, IDC_BTN_RECORD_PLAY, m_btnPlay);
    DDX_Control(pDX, IDC_BTN_RECORD_SAVE, m_btnSave);
    DDX_Control(pDX, IDC_LBL_RECORD_STATUS, m_lblStatus);
    DDX_Control(pDX, IDC_EDT_RECORD_SCRIPT, m_edtScript);
}

BEGIN_MESSAGE_MAP(CRecordDlg, CDialog)
    ON_WM_SIZE()
    ON_WM_CLOSE()
    ON_BN_CLICKED(IDC_BTN_RECORD, &CRecordDlg::OnBtnRecord)
    ON_BN_CLICKED(IDC_BTN_RECORD_PLAY, &CRecordDlg::OnBtnPlay)
    ON_BN_CLICKED(IDC_BTN_RECORD_SAVE, &CRecordDlg::OnBtnSave)
    ON_MESSAGE(WM_HOTKEY, &CRecordDlg::OnHotKey)
    ON_MESSAGE(WM_ON_SCRIPT_EXECUTE_START, &CRecordDlg::OnPlaybackStarted)
    ON_MESSAGE(WM_ON_SCRIPT_EXECUTE_END, &CRecordDlg::OnPlaybackFinished)
END_MESSAGE_MAP()

BOOL CRecordDlg::OnInitDialog() {
    CDialog::OnInitDialog();

    HICON hIcon = AfxGetApp()->LoadIcon(IDI_APP_ICON);
    if (hIcon != nullptr) {
        SetIcon(hIcon, TRUE);
        SetIcon(hIcon, FALSE);
    }

    //RegisterHotKey(m_hWnd, 1, 0, VK_F9);
    //RegisterHotKey(m_hWnd, 2, 0, VK_F10);

#ifdef _LANG_ZH_TW_
    SetWindowTextW(_T("腳本記錄器"));
    m_btnSave.SetWindowTextW(_T("保存腳本"));
#else
    SetWindowTextW(_T("Script Recorder"));
    m_btnSave.SetWindowTextW(_T("Save Script"));
#endif // _LANG_ZH_TW_

    CRect rect;
    GetClientRect(&rect);
    ResizeDlg(rect.Width(), rect.Height());
    UpdateUIState();

    return TRUE;
}

void CRecordDlg::ResizeDlg(int cx, int cy) {
    if (m_edtScript.GetSafeHwnd()) {
        m_edtScript.MoveWindow(10, 60, cx - 20, cy - 75);
    }
}

void CRecordDlg::SyncEditToBuffer() {
    CString wstr;
    m_edtScript.GetWindowTextW(wstr);

    std::wstringstream ss(wstr.GetString());
    std::wstring line;
    std::vector<std::wstring> buffer;
    while (std::getline(ss, line)) {
        if (!line.empty() && line.back() == '\r') line.pop_back();
        if (!line.empty()) buffer.push_back(line);
    }
    RecordHelper::GetInstance().SetScriptBuffer(buffer);
}

void CRecordDlg::SyncBufferToEdit() {
    std::vector<std::wstring> buffer;
    RecordHelper::GetInstance().GetScriptBuffer(buffer);

    std::wstring combined;
    for (const auto& line : buffer) {
        combined += line + L"\r\n";
    }

    m_edtScript.SetWindowText(combined.c_str());
}

void CRecordDlg::UpdateUIState() {
    if (RecordHelper::GetInstance().IsRecording()) {
#ifdef _LANG_ZH_TW_
        m_btnRecord.SetWindowTextW(_T("停止記錄"));
        m_lblStatus.SetWindowTextW(_T("狀態: 記錄中..."));
#else
        m_btnRecord.SetWindowTextW(_T("Stop Rec"));
        m_lblStatus.SetWindowTextW(_T("Status: RECORDING..."));
#endif // _LANG_ZH_TW_
        m_btnPlay.EnableWindow(FALSE);
    }
    else if (RecordHelper::GetInstance().IsPlaying()) {
#ifdef _LANG_ZH_TW_
        m_btnPlay.SetWindowTextW(_T("停止執行"));
        m_lblStatus.SetWindowTextW(_T("狀態: 執行中..."));
#else
        m_btnPlay.SetWindowTextW(_T("Stop Play"));
        m_lblStatus.SetWindowTextW(_T("Status: EXECUTING..."));
#endif // _LANG_ZH_TW_
        m_btnRecord.EnableWindow(FALSE);
    }
    else {
#ifdef _LANG_ZH_TW_
        m_btnRecord.SetWindowTextW(_T("記錄"));
        m_btnPlay.SetWindowTextW(_T("執行"));
        m_lblStatus.SetWindowTextW(_T("狀態: 無"));
#else
        m_btnRecord.SetWindowTextW(_T("Record"));
        m_btnPlay.SetWindowTextW(_T("Play"));
        m_lblStatus.SetWindowTextW(_T("Status: IDLE"));
#endif // _LANG_ZH_TW_
        m_btnRecord.EnableWindow(TRUE);
        m_btnPlay.EnableWindow(TRUE);
    }
}

void CRecordDlg::OnBtnRecord() {
    if (RecordHelper::GetInstance().IsRecording()) {
        RecordHelper::GetInstance().StopRecording();
        SyncBufferToEdit();
    } else {
        //if (RecordHelper::GetInstance().IsPlaying()) RecordHelper::GetInstance().StopPlayback();
        RecordHelper::GetInstance().StartRecording();
    }
    UpdateUIState();
}

void CRecordDlg::OnBtnPlay() {
    if (RecordHelper::GetInstance().IsPlaying()) {
        RecordHelper::GetInstance().StopPlayback();
    } else {
        if (RecordHelper::GetInstance().IsRecording()) RecordHelper::GetInstance().StopRecording();
        SyncEditToBuffer();    
        std::vector<ParsedCommand> parsedCommands;
        std::wstring errorMsg;
        int errorLine = 0;
        if (!RecordHelper::GetInstance().GetCommands(parsedCommands, errorMsg, errorLine)) {
#ifdef _LANG_ZH_TW_
            ::MessageBoxW(m_hWnd, errorMsg.c_str(), L"腳本錯誤", MB_ICONERROR | MB_OK);
#else
            ::MessageBoxW(m_hWnd, errorMsg.c_str(), L"Script Syntax Error", MB_ICONERROR | MB_OK);
#endif // _LANG_ZH_TW_
            return;
        }
        RecordHelper::GetInstance().StartPlayback(parsedCommands, [this]() {
            if (m_hWnd) ::PostMessage(m_hWnd, WM_ON_SCRIPT_EXECUTE_START, 0, 0);
            }, [this](bool hasError) {
            if (m_hWnd) ::PostMessage(m_hWnd, WM_ON_SCRIPT_EXECUTE_END, 0, 0);
        });
    }
}

void CRecordDlg::OnBtnSave() {
    SyncEditToBuffer();
    std::vector<std::wstring> lines;
    RecordHelper::GetInstance().GetScriptBuffer(lines);

#ifdef _LANG_ZH_TW_
    std::vector<std::wstring> headers = {
        L"設定開始按鍵(F12)",
        L"設定停止按鍵(None)",
        L"設定運行模式(完整單次)",
        L"",
    };
#else
    std::vector<std::wstring> headers = {
        L"SetStartKey(F12)",
        L"SetEndKey(None)",
        L"SetRunMode(FullSingle)",
        L"",
    };
#endif // _LANG_ZH_TW_

    CFileDialog fileDlg(FALSE, _T("txt"), nullptr,
        OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,
        _T("UTF-8 Text Scripts (*.txt)|*.txt|All Files (*.*)|*.*||"), this);

    if (fileDlg.DoModal() == IDOK) {
        CString filePath = fileDlg.GetPathName();

        CFile file;
        CFileException ex;

        if (!file.Open(filePath, CFile::modeCreate | CFile::modeWrite | CFile::shareExclusive, &ex)) {
            TCHAR szError[1024];
            ex.GetErrorMessage(szError, 1024);
#ifdef _LANG_ZH_TW_
            ::MessageBoxW(m_hWnd, szError, L"保存失敗", MB_ICONERROR | MB_OK);
#else
            ::MessageBoxW(m_hWnd, szError, L"Save File Failed", MB_ICONERROR | MB_OK);
#endif // _LANG_ZH_TW_
            return;
        }

        try {
            uint8_t bom[] = { 0xEF, 0xBB, 0xBF };
            uint8_t nextLine[] = { '\r', '\n' };
            file.Write(bom, sizeof(bom));

            auto combined = std::array{ std::ref(headers), std::ref(lines) };

            for (const auto& containerRef : combined) {
                for (const auto& line : containerRef.get()) {
                    auto utf8String = Utf16ToUtf8(line);
                    file.Write(utf8String.c_str(), static_cast<uint32_t>(utf8String.length()));
                    file.Write(nextLine, sizeof(nextLine));
                }
            }
        } catch (CFileException* pEx) {
            pEx->ReportError();
            pEx->Delete();
            file.Close();
            return;
        }

        file.Close();

#ifdef _LANG_ZH_TW_
        ::MessageBoxW(m_hWnd, std::format(L"腳本已保存至: {}", filePath.GetString()).c_str(), L"保存成功", MB_ICONERROR | MB_OK);
#else
        ::MessageBoxW(m_hWnd, std::format(L"Script is saved to: {}", filePath.GetString()).c_str(), L"Save Success", MB_ICONERROR | MB_OK);
#endif // _LANG_ZH_TW_
    }

}

LRESULT CRecordDlg::OnHotKey(WPARAM wParam, LPARAM lParam) {
    if (wParam == 1) OnBtnRecord();
    else if (wParam == 2) OnBtnPlay();
    return 0;
}

LRESULT CRecordDlg::OnPlaybackStarted(WPARAM, LPARAM) {
    UpdateUIState();
    return 0;
}

LRESULT CRecordDlg::OnPlaybackFinished(WPARAM, LPARAM) {
    UpdateUIState();
    return 0;
}

void CRecordDlg::OnSize(UINT nType, int cx, int cy) {
    CDialog::OnSize(nType, cx, cy);
    ResizeDlg(cx, cy);
}

void CRecordDlg::OnClose() {
    //UnregisterHotKey(m_hWnd, 1);
    //UnregisterHotKey(m_hWnd, 2);
    if (RecordHelper::GetInstance().IsRecording()) RecordHelper::GetInstance().StopRecording();
    if (RecordHelper::GetInstance().IsPlaying()) RecordHelper::GetInstance().StopPlayback();
    CDialog::OnClose();
}

void CRecordDlg::OnOK() {
    DestroyWindow();
}

void CRecordDlg::OnCancel() {
    DestroyWindow();
}

void CRecordDlg::PostNcDestroy() {
    if (m_onDestory != nullptr) m_onDestory();
    CDialog::PostNcDestroy();

    delete this; 
}
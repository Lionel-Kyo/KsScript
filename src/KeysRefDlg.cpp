#include "pch.h"
#include "KeysRefDlg.h"

CKeysRefDlg::CKeysRefDlg(CWnd* pParent, std::function<void()> onDestory) {
    m_onDestory = onDestory;
}

void CKeysRefDlg::DoDataExchange(CDataExchange* pDX) {
    CDialog::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_LIST_KEYS, m_keyList);
}

BEGIN_MESSAGE_MAP(CKeysRefDlg, CDialog)
    ON_WM_SIZE()
    ON_WM_CLOSE()
END_MESSAGE_MAP()

BOOL CKeysRefDlg::OnInitDialog() {
    CDialog::OnInitDialog();

    HICON hIcon = AfxGetApp()->LoadIcon(IDI_APP_ICON);
    if (hIcon != nullptr) {
        SetIcon(hIcon, TRUE);
        SetIcon(hIcon, FALSE);
    }

#ifdef _LANG_ZH_TW_
    SetWindowTextW(_T("腳本鍵盤按鍵說明"));
#else
    SetWindowTextW(_T("Script Keys Reference"));
#endif // _LANG_ZH_TW_

    m_keyList.SetExtendedStyle(LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);
#ifdef _LANG_ZH_TW_
    m_keyList.InsertColumn(0, _T("名稱"), LVCFMT_LEFT, 195);
    m_keyList.InsertColumn(1, _T("數值"), LVCFMT_LEFT, 195);
#else
    m_keyList.InsertColumn(0, _T("Name"), LVCFMT_LEFT, 195);
    m_keyList.InsertColumn(1, _T("Value"), LVCFMT_LEFT, 195);
#endif // _LANG_ZH_TW_

    CRect rect;
    GetClientRect(&rect);
    ResizeDlg(rect.Width(), rect.Height());
    PopulateKeyList();

    return TRUE;
}

BOOL CKeysRefDlg::PreTranslateMessage(MSG* pMsg) {
    if (CListCtrlPreTranslateMessageClipboard(pMsg, this, m_keyList)) {
        return TRUE;
    }

    return CDialog::PreTranslateMessage(pMsg);
}

void CKeysRefDlg::ResizeDlg(int cx, int cy) {
    if (m_keyList.GetSafeHwnd()) {
        m_keyList.MoveWindow(10, 10, cx - 20, cy - 25);
    }
    if (::IsWindow(m_keyList.GetSafeHwnd())) {
        ResizeKeyList();
    }
}

void CKeysRefDlg::ResizeKeyList() {
    CRect rc;
    m_keyList.GetClientRect(&rc);

    int fixedWidth = 0;

    for (int i = 0; i < 1; ++i)
        fixedWidth += m_keyList.GetColumnWidth(i);

    int width = rc.Width() - fixedWidth;

    if (width < 195) width = 195;

    m_keyList.SetColumnWidth(1, width);
}

void CKeysRefDlg::OnSize(UINT nType, int cx, int cy) {
    CDialog::OnSize(nType, cx, cy);
    ResizeDlg(cx, cy);
}

void CKeysRefDlg::OnClose() {
    CDialog::OnClose();
}

void CKeysRefDlg::OnOK() {
    DestroyWindow();
}

void CKeysRefDlg::OnCancel() {
    DestroyWindow();
}

void CKeysRefDlg::PostNcDestroy() {
    if (m_onDestory != nullptr) m_onDestory();
    CDialog::PostNcDestroy();

    delete this;
}

void CKeysRefDlg::PopulateKeyList() {
    int index = 0;

    std::vector<std::pair<std::wstring, int32_t>> orderedKeys(NAME_MATCH_KEYS.begin(), NAME_MATCH_KEYS.end());

    std::sort(orderedKeys.begin(), orderedKeys.end(), [](const auto& a, const auto& b) {
        return a.second < b.second;
    });

    for (const auto& [key, value] : orderedKeys) {
        int row = m_keyList.InsertItem(index, key.c_str());
        m_keyList.SetItemText(row, 1, std::format(L"0x{:X}", value).c_str());
        index++;
    }
}
#include "pch.h"
#include "FunctionsRefDlg.h"

CFunctionsRefDlg::CFunctionsRefDlg(CWnd* pParent, std::function<void()> onDestory) {
    m_onDestory = onDestory;
}

void CFunctionsRefDlg::DoDataExchange(CDataExchange* pDX) {
    CDialog::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_LIST_FUNCTIONS, m_functionList);
}

BEGIN_MESSAGE_MAP(CFunctionsRefDlg, CDialog)
    ON_WM_SIZE()
    ON_WM_CLOSE()
END_MESSAGE_MAP()

BOOL CFunctionsRefDlg::OnInitDialog() {
    CDialog::OnInitDialog();

    HICON hIcon = AfxGetApp()->LoadIcon(IDI_APP_ICON);
    if (hIcon != nullptr) {
        SetIcon(hIcon, TRUE);
        SetIcon(hIcon, FALSE);
    }

#ifdef _LANG_ZH_TW_
    SetWindowTextW(_T("腳本指令說明"));
#else
    SetWindowTextW(_T("Script Functions Reference"));
#endif // _LANG_ZH_TW_

    m_functionList.SetExtendedStyle(LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);
#ifdef _LANG_ZH_TW_
    m_functionList.InsertColumn(0, _T("名稱"), LVCFMT_LEFT, 180);
    m_functionList.InsertColumn(1, _T("引數"), LVCFMT_LEFT, 180);
    m_functionList.InsertColumn(2, _T("說明"), LVCFMT_LEFT, 380);
#else
    m_functionList.InsertColumn(0, _T("Name"), LVCFMT_LEFT, 180);
    m_functionList.InsertColumn(1, _T("Parameters"), LVCFMT_LEFT, 180);
    m_functionList.InsertColumn(2, _T("Explanation"), LVCFMT_LEFT, 380);
#endif // _LANG_ZH_TW_

    CRect rect;
    GetClientRect(&rect);
    ResizeDlg(rect.Width(), rect.Height());
    PopulateFunctionList();

    return TRUE;
}

BOOL CFunctionsRefDlg::PreTranslateMessage(MSG* pMsg) {
    if (CListCtrlPreTranslateMessageClipboard(pMsg, this, m_functionList)) {
        return TRUE;
    }

    return CDialog::PreTranslateMessage(pMsg);
}

void CFunctionsRefDlg::ResizeDlg(int cx, int cy) {
    if (m_functionList.GetSafeHwnd()) {
        m_functionList.MoveWindow(10, 10, cx - 20, cy - 25);
    }
    if (::IsWindow(m_functionList.GetSafeHwnd())) {
        ResizeFunctionList();
    }
}

void CFunctionsRefDlg::ResizeFunctionList() {
    CRect rc;
    m_functionList.GetClientRect(&rc);

    int fixedWidth = 0;

    for (int i = 0; i < 2; ++i)
        fixedWidth += m_functionList.GetColumnWidth(i);

    int width = rc.Width() - fixedWidth;

    if (width < 380) width = 380;

    m_functionList.SetColumnWidth(2, width);
}

void CFunctionsRefDlg::OnSize(UINT nType, int cx, int cy) {
    CDialog::OnSize(nType, cx, cy);
    ResizeDlg(cx, cy);
}

void CFunctionsRefDlg::OnClose() {
    CDialog::OnClose();
}

void CFunctionsRefDlg::OnOK() {
    DestroyWindow();
}

void CFunctionsRefDlg::OnCancel() {
    DestroyWindow();
}

void CFunctionsRefDlg::PostNcDestroy() {
    if (m_onDestory != nullptr) m_onDestory();
    CDialog::PostNcDestroy();

    delete this;
}

void CFunctionsRefDlg::PopulateFunctionList() {
    struct FuncInfo {
        CString name;
        CString parameters;
        CString explain;
    };

#ifdef _LANG_ZH_TW_
    FuncInfo items[] = {
        { _T("設定開始按鍵"), _T("(按鍵, [modifiers...])"), _T("設定腳本啟動觸發鍵。接受按鍵名稱或按鍵編號，可選的modifiers：CtrlDown、CtrlUp、ShiftDown、ShiftUp、AltDown、AltUp。") },
        { _T("設定停止按鍵"), _T("(按鍵, [modifiers...])"), _T("設定腳本停止觸發鍵。接受按鍵名稱或按鍵編號，可選的modifiers：CtrlDown、CtrlUp、ShiftDown、ShiftUp、AltDown、AltUp。") },
        { _T("設定運行模式"), _T("(模式)"), _T("設定運行模式。有效模式：單次、連續、切換。") },
        { _T("延遲"), _T("(延遲毫秒)"), _T("將腳本執行暫停指定的毫秒。") },
        { _T("滑鼠按下"), _T("(滑鼠按鍵)"), _T("按下指定的滑鼠按鍵。有效滑鼠按鍵：左、右、中、X1、X2。") },
        { _T("滑鼠放開"), _T("(滑鼠按鍵)"), _T("放開指定的滑鼠按鍵。有效滑鼠按鍵：左、右、中、X1、X2。") },
        { _T("絕對移動"), _T("(x, y)"), _T("將滑鼠游標移動至指定的絕對螢幕座標 (x, y)。") },
        { _T("相對移動"), _T("(dx, dy)"), _T("相對於目前位置，依 (dx, dy) 移動滑鼠游標。") },
        { _T("鍵盤按下"), _T("(按鍵)"), _T("按下鍵盤按鍵，接受按鍵名稱或按鍵編號。") },
        { _T("鍵盤放開"), _T("(按鍵)"), _T("放開鍵盤按鍵，接受按鍵名稱或按鍵編號。") },
        { _T("滾輪下"), _T("()"), _T("向下滾動滑鼠滾輪。此指令不需要任何引數。") },
        { _T("滾輪上"), _T("()"), _T("向上滾動滑鼠滾輪。此指令不需要任何引數。") },
        { _T("顯示座標"), _T("()"), _T("顯示目前的游標螢幕座標。此指令不需要任何引數。") },
        { _T("輸出文字"), _T("(文字, [延遲毫秒])"), _T("輸出文字，可選填字元間的延遲毫秒數。單引號 ('...') 表示字面值；雙引號 (\"...\") 表示escape sequences。") }
    };
#else
    FuncInfo items[] = {
        { _T("SetStartKey"),    _T("(key, [modifiers...])"), _T("Defines the script start trigger. Accepts a key name or VK code, with optional modifier states (CtrlDown, CtrlUp, ShiftDown, ShiftUp, AltDown, AltUp).") },
        { _T("SetEndKey"),      _T("(key, [modifiers...])"), _T("Defines the script stop trigger. Accepts a key name or VK code, with optional modifier states (CtrlDown, CtrlUp, ShiftDown, ShiftUp, AltDown, AltUp).") },
        { _T("SetRunMode"),     _T("(mode)"),                _T("Sets execution mode. Valid modes: Single, Continuous, or Switch.") },
        { _T("Delay"),          _T("(ms)"),                  _T("Pauses script execution for the specified duration in milliseconds.") },
        { _T("MouseDown"),      _T("(buttonName)"),          _T("Presses a mouse button down. Valid buttons: Left, Right, Middle, X1, X2.") },
        { _T("MouseUp"),        _T("(buttonName)"),          _T("Releases a mouse button. Valid buttons: Left, Right, Middle, X1, X2.") },
        { _T("AbsoluteMove"),   _T("(x, y)"),                _T("Moves the mouse cursor to absolute screen coordinates (x, y).") },
        { _T("RelativeMove"),   _T("(dx, dy)"),              _T("Moves the mouse cursor relative to its current position by (dx, dy).") },
        { _T("KeyboardDown"),   _T("(key)"),                 _T("Presses down a keyboard key using a key name or virtual-key (VK) code.") },
        { _T("KeyboardUp"),     _T("(key)"),                 _T("Releases a keyboard key using a key name or virtual-key (VK) code.") },
        { _T("WheelDown"),      _T("()"),                    _T("Scrolls the mouse wheel down. This command takes no arguments.") },
        { _T("WheelUp"),        _T("()"),                    _T("Scrolls the mouse wheel up. This command takes no arguments.") },
        { _T("ShowCoordinate"), _T("()"),                    _T("Displays the current cursor screen coordinates. This command takes no arguments.") },
        { _T("OutputText"),     _T("(text, [delayMs])"),     _T("Outputs a text string with an optional delay in milliseconds between characters. Single quotes ('...') are literal; double quotes (\"...\") support escape sequences.") }
    };
#endif // _LANG_ZH_TW_

    int index = 0;
    for (const auto& item : items) {
        int row = m_functionList.InsertItem(index, item.name);
        m_functionList.SetItemText(row, 1, item.parameters);
        m_functionList.SetItemText(row, 2, item.explain);

        index++;
    }
}
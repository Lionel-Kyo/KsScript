#include "pch.h"
#include "DarkDialog.h"

#pragma comment(lib, "uxtheme.lib")

CDarkDialogBase::CDarkDialogBase(UINT nIDTemplate, CWnd* pParent) : CDialogEx(nIDTemplate, pParent) {
    m_containerBrush.CreateSolidBrush(DarkTheme::ContainerBackColor);
    m_subtleBrush.CreateSolidBrush(DarkTheme::ComponentSubtleBackColor);
}

BOOL CDarkDialogBase::OnInitDialog() {
    CDialogEx::OnInitDialog();
    DarkTheme::EnableDarkTitleBar(GetSafeHwnd());
    SetBackgroundColor(DarkTheme::ContainerBackColor);
    return TRUE;
}

HBRUSH CDarkDialogBase:: OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor) {
    if (nCtlColor == CTLCOLOR_STATIC) {
        pDC->SetTextColor(DarkTheme::ForeColor);
        pDC->SetBkMode(TRANSPARENT);
        return m_containerBrush;
    }
    if (nCtlColor == CTLCOLOR_EDIT) {
        pDC->SetTextColor(DarkTheme::ForeColor);
        pDC->SetBkColor(DarkTheme::ComponentSubtleBackColor);
        return m_subtleBrush;
    }
    return CDialogEx::OnCtlColor(pDC, pWnd, nCtlColor);
}

BEGIN_MESSAGE_MAP(CDarkDialogBase, CDialogEx)
    ON_WM_CTLCOLOR()
END_MESSAGE_MAP()
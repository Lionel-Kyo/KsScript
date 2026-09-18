#pragma once
#include <afxwin.h>
#include <afxdialogex.h>
#include <uxtheme.h>
#include "ThemeColors.h"

#pragma comment(lib, "uxtheme.lib")

class CDarkDialogBase : public CDialogEx {
protected:
    CBrush m_containerBrush;
    CBrush m_subtleBrush;

public:
    CDarkDialogBase(UINT nIDTemplate, CWnd* pParent = nullptr);

    virtual BOOL OnInitDialog() override;

    afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);

    DECLARE_MESSAGE_MAP()
};
#pragma once
#include <afxwin.h>
#include "ThemeColors.h"

class CDarkButton : public CButton {
public:
    CDarkButton();
    void SetChecked(bool checked);
    bool IsChecked();

protected:
    bool m_isHovered;
    bool m_isTracking;
    bool m_isChecked;

    virtual void DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct) override;
    afx_msg void OnMouseMove(UINT nFlags, CPoint point);
    afx_msg LRESULT OnMouseLeave(WPARAM wParam, LPARAM lParam);
    DECLARE_MESSAGE_MAP()
};
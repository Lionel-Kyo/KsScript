#include "pch.h"
#include "DarkButton.h"

BEGIN_MESSAGE_MAP(CDarkButton, CButton)
    ON_WM_MOUSEMOVE()
    ON_MESSAGE(WM_MOUSELEAVE, OnMouseLeave)
END_MESSAGE_MAP()

CDarkButton::CDarkButton() : m_isHovered(false), m_isTracking(false), m_isChecked(false) {}

void CDarkButton::SetChecked(bool checked) {
    m_isChecked = checked;
    Invalidate();
}

bool CDarkButton::IsChecked() {
    return m_isChecked;
}

void CDarkButton::OnMouseMove(UINT nFlags, CPoint point) {
    if (!m_isTracking) {
        TRACKMOUSEEVENT tme = { sizeof(TRACKMOUSEEVENT), TME_LEAVE, m_hWnd, 0 };
        m_isTracking = TrackMouseEvent(&tme) != FALSE;
        m_isHovered = true;
        Invalidate();
    }
    CButton::OnMouseMove(nFlags, point);
}

LRESULT CDarkButton::OnMouseLeave(WPARAM wParam, LPARAM lParam) {
    m_isTracking = false;
    m_isHovered = false;
    Invalidate();
    return 0;
}

void CDarkButton::DrawItem(LPDRAWITEMSTRUCT lpDIS) {
    CDC* pDC = CDC::FromHandle(lpDIS->hDC);
    CRect rect = lpDIS->rcItem;
    BOOL isPressed = (lpDIS->itemState & ODS_SELECTED);
    BOOL isFocused = (lpDIS->itemState & ODS_FOCUS);

    // Color resolution based on state
    COLORREF backColor = DarkTheme::ComponentBackColor;
    COLORREF textColor = DarkTheme::ForeColor;
    COLORREF borderColor = DarkTheme::BorderColor;

    if (m_isChecked) {
        backColor = isPressed ? DarkTheme::ButtonCheckedPressColor :
                    m_isHovered ? DarkTheme::ButtonCheckedHoverColor : DarkTheme::ButtonCheckedBackColor;
        textColor = DarkTheme::ButtonCheckedForeColor;
        borderColor = isFocused ? DarkTheme::ButtonCheckedFocusColor : DarkTheme::SubtleBorderColor;
    } else {
        if (isPressed) backColor = DarkTheme::PressColor;
        else if (m_isHovered) backColor = DarkTheme::HoverColor;
        else if (isFocused) backColor = DarkTheme::FocusColor;

        if (isFocused) borderColor = DarkTheme::PrimaryFocusColor;
    }

    // Fill background
    pDC->FillSolidRect(rect, backColor);

    // Draw border
    CPen pen(PS_SOLID, 1, borderColor);
    CPen* pOldPen = pDC->SelectObject(&pen);
    CBrush* pOldBrush = (CBrush*)pDC->SelectStockObject(NULL_BRUSH);
    pDC->Rectangle(rect);
    pDC->SelectObject(pOldPen);
    pDC->SelectObject(pOldBrush);

    // Draw Text
    CString text;
    GetWindowText(text);
    pDC->SetBkMode(TRANSPARENT);
    pDC->SetTextColor(textColor);
    pDC->DrawText(text, rect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
}
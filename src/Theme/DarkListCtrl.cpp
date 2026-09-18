#include "pch.h"
#include "DarkListCtrl.h"

BEGIN_MESSAGE_MAP(CDarkListCtrl, CListCtrl)
    ON_NOTIFY(NM_CUSTOMDRAW, 0, OnHeaderCustomDraw)            // Intercept child header notifications
    ON_NOTIFY_REFLECT(NM_CUSTOMDRAW, OnItemCustomDraw)          // Intercept list item selection colors
END_MESSAGE_MAP()

void CDarkListCtrl::PreSubclassWindow() {
    CListCtrl::PreSubclassWindow();
    ApplyDarkTheme();
}

void CDarkListCtrl::ApplyDarkTheme() {
    DWORD dwStyle = GetExtendedStyle();
    dwStyle |= (LVS_EX_FULLROWSELECT | LVS_EX_DOUBLEBUFFER);
    dwStyle &= ~LVS_EX_GRIDLINES; // Turn off native light gridlines
    SetExtendedStyle(dwStyle);

    // Set list background and text colors
    SetBkColor(DarkTheme::ComponentSubtleBackColor);
    SetTextBkColor(DarkTheme::ComponentSubtleBackColor);
    SetTextColor(DarkTheme::ForeColor);

    // Apply UxTheme dark mode (activates dark scrollbars)
    DarkTheme::EnableControlDarkMode(GetSafeHwnd());

    // Apply dark mode to header control
    if (CHeaderCtrl* pHeader = GetHeaderCtrl()) {
        DarkTheme::EnableControlDarkMode(pHeader->GetSafeHwnd());
    }

    // SetWindowTheme(GetSafeHwnd(), L"Explorer", NULL);
}

void CDarkListCtrl::SetPostHeaderCustomDraw(std::function<void(NMHDR* pNMHDR, LRESULT* pResult)> postHeaderCustomDraw) {
    m_postHeaderCustomDraw = postHeaderCustomDraw;
}

void CDarkListCtrl::SetPostItemCustomDraw(std::function<void(NMHDR* pNMHDR, LRESULT* pResult)> postItemCustomDraw) {
    m_postItemCustomDraw = postItemCustomDraw;
}

void CDarkListCtrl::OnHeaderCustomDraw(NMHDR* pNMHDR, LRESULT* pResult) {
    LPNMCUSTOMDRAW pNMCDRW = reinterpret_cast<LPNMCUSTOMDRAW>(pNMHDR);
    *pResult = CDRF_DODEFAULT;

    CHeaderCtrl* pHeader = GetHeaderCtrl();
    if (!pHeader || pNMHDR->hwndFrom != pHeader->GetSafeHwnd()) return;

    switch (pNMCDRW->dwDrawStage) {
    case CDDS_PREPAINT:
        *pResult = CDRF_NOTIFYITEMDRAW;
        break;

    case CDDS_ITEMPREPAINT: {
        CDC* pDC = CDC::FromHandle(pNMCDRW->hdc);
        CRect rect(pNMCDRW->rc);

        // Fill background
        pDC->FillSolidRect(rect, DarkTheme::ComponentSubtleBackColor);

        // Header text
        TCHAR text[256] = { 0 };
        HDITEM hdi = { 0 };
        hdi.mask = HDI_TEXT;
        hdi.pszText = text;
        hdi.cchTextMax = 256;
        pHeader->GetItem(static_cast<int>(pNMCDRW->dwItemSpec), &hdi);

        pDC->SetBkMode(TRANSPARENT);
        pDC->SetTextColor(DarkTheme::ForeColor);
        rect.left += 6;
        pDC->DrawText(text, rect, DT_LEFT | DT_VCENTER | DT_SINGLELINE);

        // Right divider line
        CPen pen(PS_SOLID, 1, DarkTheme::SeparatorColor);
        CPen* pOldPen = pDC->SelectObject(&pen);
        pDC->MoveTo(rect.right - 1, rect.top);
        pDC->LineTo(rect.right - 1, rect.bottom);
        pDC->SelectObject(pOldPen);

        *pResult = CDRF_SKIPDEFAULT;
        break;
    }
    }

    if (m_postHeaderCustomDraw != nullptr) m_postHeaderCustomDraw(pNMHDR, pResult);
}

void CDarkListCtrl::OnItemCustomDraw(NMHDR* pNMHDR, LRESULT* pResult) {
    auto* pLVCD = reinterpret_cast<NMLVCUSTOMDRAW*>(pNMHDR);
    *pResult = CDRF_DODEFAULT;

    switch (pLVCD->nmcd.dwDrawStage) {
    case CDDS_PREPAINT:
        *pResult = CDRF_NOTIFYITEMDRAW;
        break;

    case CDDS_ITEMPREPAINT:
        // Request subitem notifications AND row-level post-paint notifications
        *pResult = CDRF_NOTIFYSUBITEMDRAW | CDRF_NOTIFYPOSTPAINT;
        break;

    case CDDS_ITEMPREPAINT | CDDS_SUBITEM: {
        const int row = static_cast<int>(pLVCD->nmcd.dwItemSpec);
        const int column = pLVCD->iSubItem;

        // Apply dark selection/text colors
        if (pLVCD->nmcd.uItemState & CDIS_SELECTED) {
            pLVCD->clrText = DarkTheme::ForeColor;
            pLVCD->clrTextBk = DarkTheme::FocusColor;
        }
        else {
            pLVCD->clrText = DarkTheme::ForeColor;
            pLVCD->clrTextBk = DarkTheme::ComponentSubtleBackColor;
        }

        // Custom draw checkbox for Column 0
        CHeaderCtrl* pHeader = GetHeaderCtrl();
        int nColumnCount = 0;
        if (pHeader != nullptr) nColumnCount = pHeader->GetItemCount();
        for (int32_t i = 0; i < nColumnCount; ++i) {
            CRect cellRect;
            GetSubItemRect(row, column, LVIR_BOUNDS, cellRect);
            cellRect.right = cellRect.left + GetColumnWidth(0);

            CDC* dc = CDC::FromHandle(pLVCD->nmcd.hdc);
            COLORREF bgCol = (pLVCD->nmcd.uItemState & CDIS_SELECTED) ? DarkTheme::FocusColor : DarkTheme::ComponentSubtleBackColor;
            dc->FillSolidRect(cellRect, bgCol);
            *pResult = CDRF_SKIPDEFAULT;
        }

        if (column == 0) {
            CRect cellRect;
            GetSubItemRect(row, column, LVIR_BOUNDS, cellRect);
            cellRect.right = cellRect.left + GetColumnWidth(0);

            CDC* dc = CDC::FromHandle(pLVCD->nmcd.hdc);
            COLORREF bgCol = (pLVCD->nmcd.uItemState & CDIS_SELECTED) ? DarkTheme::FocusColor : DarkTheme::ComponentSubtleBackColor;
            dc->FillSolidRect(cellRect, bgCol);

            constexpr int checkSize = 16;
            const int x = cellRect.left + (cellRect.Width() - checkSize) / 2;
            const int y = cellRect.top + (cellRect.Height() - checkSize) / 2;
            CRect checkRect(x, y, x + checkSize, y + checkSize);

            const bool checked = GetCheck(row) != FALSE;

            HTHEME hTheme = OpenThemeData(m_hWnd, L"Button");
            if (hTheme) {
                int stateId = checked ? CBS_CHECKEDNORMAL : CBS_UNCHECKEDNORMAL;
                DrawThemeBackground(hTheme, dc->GetSafeHdc(), BP_CHECKBOX, stateId, &checkRect, nullptr);
                CloseThemeData(hTheme);
            }
            else {
                UINT state = DFCS_BUTTONCHECK;
                if (checked) state |= DFCS_CHECKED;
                dc->DrawFrameControl(&checkRect, DFC_BUTTON, state);
            }

            *pResult = CDRF_SKIPDEFAULT;
            return;
        }

        *pResult = CDRF_NEWFONT;
        break;
    }

                                         // Fires ONCE per row after all columns are painted
    case CDDS_ITEMPOSTPAINT: {
        const int row = static_cast<int>(pLVCD->nmcd.dwItemSpec);
        CDC* dc = CDC::FromHandle(pLVCD->nmcd.hdc);

        CPen pen(PS_SOLID, 1, DarkTheme::SeparatorColor); // RGB(51, 51, 51)
        CPen* pOldPen = dc->SelectObject(&pen);

        // 1. Draw horizontal row divider line at the bottom
        CRect rowRect;
        GetItemRect(row, rowRect, LVIR_BOUNDS);
        dc->MoveTo(rowRect.left, rowRect.bottom - 1);
        dc->LineTo(rowRect.right, rowRect.bottom - 1);

        // 2. Draw vertical column divider lines
        CHeaderCtrl* pHeader = GetHeaderCtrl();
        const int numCols = pHeader ? pHeader->GetItemCount() : 0;

        for (int col = 0; col < numCols; ++col) {
            CRect colRect;
            GetSubItemRect(row, col, LVIR_BOUNDS, colRect);
            if (col == 0) {
                colRect.right = colRect.left + GetColumnWidth(0);
            }

            dc->MoveTo(colRect.right - 1, colRect.top);
            dc->LineTo(colRect.right - 1, colRect.bottom);
        }

        dc->SelectObject(pOldPen);
        *pResult = CDRF_DODEFAULT;
        break;
    }
    }
}
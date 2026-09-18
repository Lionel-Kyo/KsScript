#pragma once
#include <afxwin.h>
#include <afxcmn.h>
#include <uxtheme.h>
#include <functional>
#include "ThemeColors.h"

class CDarkListCtrl : public CListCtrl {
public:
    CDarkListCtrl() = default;
    virtual ~CDarkListCtrl() = default;

    void ApplyDarkTheme();

    void SetPostHeaderCustomDraw(std::function<void(NMHDR* pNMHDR, LRESULT* pResult)> postHeaderCustomDraw);
    void SetPostItemCustomDraw(std::function<void(NMHDR* pNMHDR, LRESULT* pResult)> postItemCustomDraw);

protected:
    virtual void PreSubclassWindow() override;

    afx_msg void OnHeaderCustomDraw(NMHDR* pNMHDR, LRESULT* pResult);
    afx_msg void OnItemCustomDraw(NMHDR* pNMHDR, LRESULT* pResult);

    DECLARE_MESSAGE_MAP()
private:
    std::function<void(NMHDR* pNMHDR, LRESULT* pResult)> m_postHeaderCustomDraw;
    std::function<void(NMHDR* pNMHDR, LRESULT* pResult)> m_postItemCustomDraw;
};
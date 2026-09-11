#pragma once
#define _AFXDLL
#include <afxwin.h>
#include <afxext.h>
#include <functional>
#include <format>
#include "Resource.h"
#include "Utils.h"

class CKeysRefDlg : public CDialog {
public:
    CKeysRefDlg(CWnd* pParent = nullptr, std::function<void()> onDestory = nullptr);

    enum { IDD = IDD_KEYS_REF_DIALOG };

protected:
    virtual void DoDataExchange(CDataExchange* pDX) override;
    virtual BOOL OnInitDialog() override;
    virtual BOOL PreTranslateMessage(MSG* pMsg) override;

    void ResizeDlg(int cx, int cy);
    void ResizeKeyList();

    CListCtrl m_keyList;
    void PopulateKeyList();

    virtual void OnOK() override;
    virtual void OnCancel() override;
    virtual void PostNcDestroy() override;

    afx_msg void OnSize(UINT nType, int cx, int cy);
    afx_msg void OnClose();

    DECLARE_MESSAGE_MAP()

private:
    std::function<void()> m_onDestory;
};
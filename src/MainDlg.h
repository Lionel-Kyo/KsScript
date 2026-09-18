#pragma once
#define _AFXDLL
#include <afxwin.h>
#include <afxext.h>
#include <afxcmn.h>
#include <afxdialogex.h>
#include <vector>
#include <string>
#include <format>
#include <filesystem>
#include "Resource.h"
#include "MainHelper.h"
#include "Utils.h"
#include "RecordDlg.h"
#include "FunctionsRefDlg.h"
#include "KeysRefDlg.h"
#include "Theme/DarkDialog.h"
#include "Theme/DarkButton.h"
#include "Theme/DarkListCtrl.h"

class CMainDlg : public CDarkDialogBase {
public:
    CMainDlg(CWnd* pParent = nullptr);

    enum { IDD = IDD_MAIN_DIALOG };

protected:
    CDarkButton m_btnLoadScripts;
    CDarkButton m_btnOpenRecordDlg;
    CDarkButton m_btnOpenFunctionsRefDlg;
    CDarkButton m_btnOpenKeysRefDlg;
    CDarkListCtrl m_lstScripts;
    CRecordDlg* m_recordDlg = nullptr;
    CFunctionsRefDlg* m_functionsRefDlg = nullptr;
    CKeysRefDlg* m_keysRefDlg = nullptr;

    void ResizeDlg(int cx, int cy);
    void ResizeScriptListColumns();

    virtual void DoDataExchange(CDataExchange* pDX) override;
    virtual BOOL OnInitDialog() override;
    virtual BOOL PreTranslateMessage(MSG* pMsg) override;

    bool LoadScriptEntry(const std::wstring& filePath, std::wstring& outErrorContent, std::wstring& outErrorTitle, ScriptEntry& outEntry);

    afx_msg void OnBtnOpenRecordDlg();
    afx_msg void OnBtnOpenFunctionsRefDlg();
    afx_msg void OnBtnOpenKeysRefDlg();
    afx_msg void OnBtnLoadScripts();
    afx_msg void OnNMCustomdrawLstScripts(NMHDR* pNMHDR, LRESULT* pResult);
    afx_msg void OnHeaderItemChanged(NMHDR* pNMHDR, LRESULT* pResult);
    afx_msg void OnNMClickLstScripts(NMHDR* pNMHDR, LRESULT* pResult);
    afx_msg void OnNMRClickLstScripts(NMHDR* pNMHDR, LRESULT* pResult);
    afx_msg void OnLvnItemchangedLstScripts(NMHDR* pNMHDR, LRESULT* pResult);
    afx_msg void OnSize(UINT nType, int cx, int cy);
    afx_msg void OnClose();

    DECLARE_MESSAGE_MAP()
};
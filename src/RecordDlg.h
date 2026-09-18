#pragma once
#define _AFXDLL
#include <afxwin.h>
#include <afxext.h>
#include <functional>
#include <sstream>
#include <array>
#include "Resource.h"
#include "Utils.h"
#include "RecordHelper.h"

class CRecordDlg : public CDialog {
public:
    CRecordDlg(CWnd* pParent = nullptr, std::function<void()> onDestory = nullptr);

    enum { IDD = IDD_RECORD_DIALOG };

protected:
    virtual void DoDataExchange(CDataExchange* pDX) override;
    virtual BOOL OnInitDialog() override;

    void ResizeDlg(int cx, int cy);

    CButton m_btnRecord;
    CButton m_btnPlay;
    CButton m_btnSave;
    CStatic m_lblStatus;
    CEdit m_edtScript;

    void SyncEditToBuffer();
    void SyncBufferToEdit();
    void UpdateUIState();

    virtual void OnOK() override;
    virtual void OnCancel() override;
    virtual void PostNcDestroy() override;

    afx_msg void OnBtnRecord();
    afx_msg void OnBtnPlay();
    afx_msg void OnBtnSave();
    afx_msg LRESULT OnHotKey(WPARAM wParam, LPARAM lParam);
    afx_msg LRESULT OnPlaybackStarted(WPARAM wParam, LPARAM lParam);
    afx_msg LRESULT OnPlaybackFinished(WPARAM wParam, LPARAM lParam);
    afx_msg void OnSize(UINT nType, int cx, int cy);
    afx_msg void OnClose();

    DECLARE_MESSAGE_MAP()
private:
    std::function<void()> m_onDestory;
};
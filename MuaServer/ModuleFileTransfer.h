#pragma once
#include "SocketClient.h"


// CModuleFileTransfer 对话框

class CModuleFileTransfer : public CDialogEx
{
	DECLARE_DYNAMIC(CModuleFileTransfer)
public:
	CClient*				m_pClient;
	//CSocketClient*				m_pChildSocketClient;

public:
	CModuleFileTransfer(CWnd* pParent = nullptr, CClient * pClient = nullptr);   // 标准构造函数
	virtual ~CModuleFileTransfer();

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DIALOG1 };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedButton5();
	CEdit m_EditDownloadLocalPath;
	afx_msg void OnBnClickedButton6();
	CEdit m_EditUploadLocalPath;
	CEdit m_EditUploadRemotePath;
	CEdit m_EditDownloadRemotePath;
	afx_msg void OnBnClickedUpload();
	afx_msg void OnBnClickedDownload();

	void OnOK();
	BOOL PreTranslateMessage(MSG* pMsg);
};

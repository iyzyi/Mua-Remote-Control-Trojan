#pragma once

#include "ClientManage.h"
#include "ModuleManage.h"

// CShellRemote 对话框

class CShellRemote : public CDialogEx, public CModule
{
	DECLARE_DYNAMIC(CShellRemote)

public:
	CShellRemote(CWnd* pParent = nullptr, CClient* pClient = nullptr);   // 标准构造函数
	virtual ~CShellRemote();

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DIALOG2 };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()


public:
	//CClient*		m_pClient;			基类有这个成员了

public:
	// 重写虚函数
	void OnRecvChildSocketClientPacket(CPacket* pPacket);

public:
	VOID ShellOpen();
	afx_msg void OnEnChangeEdit1();

	//void Start();
	CEdit m_EditResult;
	CEdit m_EditCommand;
	afx_msg void OnBnClickedButton1();
};




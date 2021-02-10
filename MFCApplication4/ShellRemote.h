#pragma once

#include "ClientManage.h"

// CShellRemote 对话框

class CShellRemote : public CDialogEx
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
};

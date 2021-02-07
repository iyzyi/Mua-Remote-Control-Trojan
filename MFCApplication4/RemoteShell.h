#pragma once


// RemoteShell 对话框

class RemoteShell : public CDialogEx
{
	DECLARE_DYNAMIC(RemoteShell)

public:
	RemoteShell(CWnd* pParent = nullptr);   // 标准构造函数
	virtual ~RemoteShell();

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DIALOG1 };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
};

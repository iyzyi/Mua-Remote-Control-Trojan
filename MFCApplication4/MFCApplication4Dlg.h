
// MFCApplication4Dlg.h: 头文件
//

#pragma once



#define WM_RECV_LOGIN_PACKET (WM_USER+100) 
#define WM_CLIENT_DISCONNECT (WM_USER+101)



// CMFCApplication4Dlg 对话框
class CMFCApplication4Dlg : public CDialogEx
{
// 构造
public:
	CMFCApplication4Dlg(CWnd* pParent = nullptr);	// 标准构造函数

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_MFCAPPLICATION4_DIALOG };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 支持


// 实现
protected:
	HICON m_hIcon;

	// 生成的消息映射函数
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()

public:
	CListCtrl m_ListCtrl;
	afx_msg void OnLvnItemchangedList2(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnEnChangeEdit1();
	afx_msg void OnBnClickedButton1();
	afx_msg void OnBnClickedButton2();
	CEdit m_EditIpAddress;
	CEdit m_EditPort;
	CButton m_ButtonStartSocketServer;
	CButton m_ButtonStopSocketServer;


public:
	// 这里必须是static，不然ManageRecvPacket实参与 "NOTIFYPROC" 类型的形参不兼容。
	// 而且static只能在声明中添加，不能在定义中添加哦。
	static void CALLBACK ManageRecvPacket(CPacket *Packet);

protected:
	afx_msg LRESULT OnRecvLoginPacket(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnClientDisconnect(WPARAM wParam, LPARAM lParam);

public:
	void OnOK();
	BOOL PreTranslateMessage(MSG* pMsg);
	afx_msg void OnRClickMenu(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnTouchDisconnectClient();
	afx_msg void OnOpenRemoteShell();
};
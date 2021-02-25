
// MFCApplication4Dlg.h: 头文件
//

#pragma once

#include "Packet.h"


#define WM_RECV_LOGIN_PACKET (WM_USER+100) 
#define WM_SOCKET_CLIENT_DISCONNECT (WM_USER+101)

#define WM_RECV_SHELL_CONNECT_PACKET (WM_USER+200)


#define WM_RECV_CHILD_SOCKET_CLIENT_PACKET (WM_USER+250)
#define WM_RECV_MAIN_SOCKET_CLIENT_PACKET (WM_USER+251)


// 收到被控端发来的CONNECT包时，说明成功建立了相应的子socket连接
#define WM_FILE_UPLOAD_CONNECT_SUCCESS (WM_USER+252)			
#define WM_FILE_DOWNLOAD_CONNECT_SUCCESS (WM_USER+253)



//class CPacket;


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
	//static void CALLBACK MainSocketRecvPacket(CPacket *Packet);
	//static void CALLBACK ChildSocketRecvPacket(CPacket *Packet);

protected:
	afx_msg LRESULT OnRecvLoginPacket(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnSocketClientDisconnect(WPARAM wParam, LPARAM lParam);

	//afx_msg LRESULT OnRecvShellConnectPacket(WPARAM wParam, LPARAM lParam);		// 收到SHELL_CONNECT封包时创建相应的对话框
public:
	afx_msg LRESULT OnPostMsgRecvChildSocketClientPacket(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnPostMsgRecvMainSocketClientPacket(WPARAM wParam, LPARAM lParam);


	BOOL ProcessConnectPacket(CPacket* pPacket);




public:
	void OnOK();
	BOOL PreTranslateMessage(MSG* pMsg);
	afx_msg void OnRClickMenu(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnTouchDisconnectClient();
	afx_msg void OnOpenRemoteShell();
	afx_msg void OnTouchTestEcho();


	void ProcessRClickSelectCommand(COMMAND_ID Command, PBYTE pbPacketBody, DWORD dwPacketBodyLength);
	afx_msg void OnTestFileUpload();
	afx_msg void OnTestFileDownload();
	afx_msg void OnTouchFileTransfer();
};




DWORD WINAPI WaitChildSocketCloseThreadFunc(LPVOID lparam);
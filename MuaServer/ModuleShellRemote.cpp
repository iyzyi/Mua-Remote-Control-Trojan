// ShellRemote.cpp: 实现文件
//

#include "pch.h"
#include "ModuleShellRemote.h"
#include "afxdialogex.h"
#include "resource.h"
#include "MuaServer.h"


// CShellRemote 对话框

IMPLEMENT_DYNAMIC(CShellRemote, CDialogEx)

CShellRemote::CShellRemote(CWnd* pParent /*=nullptr*/, CSocketClient* pClient /*= nullptr*/)
	: CDialogEx(IDD_DIALOG2, pParent), CModule(pClient)
{
	this->Create(IDD_DIALOG2, GetDesktopWindow());
	this->ShowWindow(SW_SHOW);
	
	WCHAR pszTitle[64];
	StringCbPrintf(pszTitle, 64, L"远程SHELL    %s:%d\n", pClient->m_lpszIpAddress, pClient->m_wPort);
	this->SetWindowText(pszTitle);
	
	// 修改焦点到编辑框上
	GetDlgItem(IDC_EDIT2)->SetFocus();

	// 提示框
	m_MyTip.Create(this);
	m_MyTip.AddTool(GetDlgItem(IDC_BUTTON1), L"直接在编辑框中按下回车即可发送SHELL，无需额外点击此按钮~");
	//m_MyTip.SetDelayTime(200);					//设置延迟
	m_MyTip.SetTipTextColor(RGB(0, 0, 255));	//设置提示文本的颜色
	m_MyTip.SetTipBkColor(RGB(255, 255, 255));	//设置提示框的背景颜色
	m_MyTip.Activate(TRUE);						//设置是否启用提示
	m_MyTip.SetDelayTime(TTDT_INITIAL, 10);		//鼠标指向多久后显示提示，毫秒
	m_MyTip.SetDelayTime(TTDT_AUTOPOP, 30000);	//鼠标保持指向，提示显示多久，毫秒

	m_dwBufferTail = 0;
}

CShellRemote::~CShellRemote()
{
}

void CShellRemote::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_EDIT1, m_EditResult);
	DDX_Control(pDX, IDC_EDIT2, m_EditCommand);
}


BEGIN_MESSAGE_MAP(CShellRemote, CDialogEx)
	ON_EN_CHANGE(IDC_EDIT1, &CShellRemote::OnEnChangeEdit1)
	ON_BN_CLICKED(IDC_BUTTON1, &CShellRemote::OnBnClickedButton1)
	ON_WM_CLOSE()								// 不写这个不会触发自己重写的OnClose()
END_MESSAGE_MAP()


// CShellRemote 消息处理程序


void CShellRemote::OnEnChangeEdit1()
{
	// TODO:  如果该控件是 RICHEDIT 控件，它将不
	// 发送此通知，除非重写 CDialogEx::OnInitDialog()
	// 函数并调用 CRichEditCtrl().SetEventMask()，
	// 同时将 ENM_CHANGE 标志“或”运算到掩码中。

	// TODO:  在此添加控件通知处理程序代码
}


// 要想改焦点，仅仅设置了SetFocus()那是不够的，还需要将对话框中的OnInitDialog的最后那句return设置为"FALSE";
BOOL CShellRemote::OnInitDialog() {
	CDialogEx::OnInitDialog();
	return false;
}


// 执行命令
void CShellRemote::OnBnClickedButton1()
{
	WCHAR pbCommand[256];
	m_EditCommand.GetWindowText(pbCommand, 256);
	DWORD dwCommandLength = (wcslen(pbCommand) + 1) * 2;
	theApp.m_Server.SendPacket(m_pSocketClient, SHELL_EXECUTE, (PBYTE)pbCommand, dwCommandLength);

	m_EditCommand.SetWindowTextW(L"");
}





// 重写虚函数
void CShellRemote::OnRecvChildSocketClientPacket(CPacket* pPacket) {
	CSocketClient* pSocketClient = pPacket->m_pSocketClient;
	
	switch (pPacket->m_PacketHead.wCommandId) {

	case SHELL_EXECUTE: 
		break;

	case SHELL_EXECUTE_RESULT:
		OnRecvPacketShellRemoteExecute(pPacket);
		break;

	case SHELL_EXECUTE_RESULT_OVER:
		OnRecvPacketShellRemoteExecute(pPacket);
		break;

	case SHELL_CLOSE:
		SendMessage(WM_CLOSE);
		break;
	}

	delete pPacket;
}


void CShellRemote::OnOK()
{
	//什么也不写
}

//然后重载PreTranslateMessage函数  
//把ESC键的消息，用RETURN键的消息替换，这样，按ESC的时候，也会执行刚才的OnOK函数  
BOOL CShellRemote::PreTranslateMessage(MSG* pMsg)
{
	// 按下回车键并且焦点在输入执行命令的那个编辑框上 等于 按下执行命令按钮
	if (pMsg->message == WM_KEYDOWN && pMsg->wParam == VK_RETURN && GetFocus() == GetDlgItem(IDC_EDIT2)) {
		OnBnClickedButton1();
	}

	// 提示框相关
	if (pMsg->message == WM_MOUSEMOVE) {
		m_MyTip.RelayEvent(pMsg);
	}

	if (pMsg->message == WM_KEYDOWN && pMsg->wParam == VK_ESCAPE)
	{
		pMsg->wParam = VK_RETURN;   //将ESC键的消息替换为回车键的消息，这样，按ESC的时候  
									//也会去调用OnOK函数，而OnOK什么也不做，这样ESC也被屏蔽  
	}
	return   CDialog::PreTranslateMessage(pMsg);
}


void CShellRemote::OnClose() {

	DWORD dwConnectId = m_pSocketClient->m_dwConnectId;
	theApp.m_Server.m_pTcpPackServer->Disconnect(dwConnectId);		// 断开这条子socket

	CDialogEx::OnClose();
}


VOID CShellRemote::OnRecvPacketShellRemoteExecute(CPacket* pPacket) {
	DWORD dwWideCharLength = MultiByteToWideChar(CP_ACP, 0, (CHAR*)pPacket->m_pbPacketBody, pPacket->m_dwPacketBodyLength, NULL, 0);
	WCHAR* pszWideCharTemp = new WCHAR[dwWideCharLength];
	MultiByteToWideChar(CP_ACP, 0, (CHAR*)pPacket->m_pbPacketBody, pPacket->m_dwPacketBodyLength, pszWideCharTemp, dwWideCharLength);

	if (dwWideCharLength > COMMAND_RESULT_BUFFER_LENGTH) {
		memcpy(m_pszWideChar, pszWideCharTemp + (dwWideCharLength - COMMAND_RESULT_BUFFER_LENGTH), COMMAND_RESULT_BUFFER_LENGTH * 2);
		m_dwBufferTail = COMMAND_RESULT_BUFFER_LENGTH;
	}
	else if (m_dwBufferTail + dwWideCharLength <= COMMAND_RESULT_BUFFER_LENGTH) {
		memcpy(m_pszWideChar + m_dwBufferTail, pszWideCharTemp, dwWideCharLength * 2);
		m_dwBufferTail += dwWideCharLength;
	}
	else {
		memcpy(m_pszWideChar, m_pszWideChar + (m_dwBufferTail + dwWideCharLength - COMMAND_RESULT_BUFFER_LENGTH), (COMMAND_RESULT_BUFFER_LENGTH - dwWideCharLength) * 2);
		memcpy(m_pszWideChar + (COMMAND_RESULT_BUFFER_LENGTH - dwWideCharLength), pszWideCharTemp, dwWideCharLength * 2);
		m_dwBufferTail = COMMAND_RESULT_BUFFER_LENGTH;
	}
	m_pszWideChar[m_dwBufferTail] = 0;		// 字符串以0结尾

	delete[] pszWideCharTemp;
	pszWideCharTemp = nullptr;

	m_EditResult.SetWindowText(m_pszWideChar);
	m_EditResult.PostMessage(WM_VSCROLL, SB_BOTTOM, 0);
}
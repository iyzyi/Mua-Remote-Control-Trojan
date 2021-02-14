// ShellRemote.cpp: 实现文件
//

#include "pch.h"
#include "ModuleShellRemote.h"
#include "afxdialogex.h"
#include "resource.h"
#include "MFCApplication4.h"


// CShellRemote 对话框

IMPLEMENT_DYNAMIC(CShellRemote, CDialogEx)

CShellRemote::CShellRemote(CWnd* pParent /*=nullptr*/, CClient* pClient /*= nullptr*/)
	: CDialogEx(IDD_DIALOG2, pParent), CModule(pClient)
{
	//m_pClient = pClient;

	this->Create(IDD_DIALOG2, GetDesktopWindow());
	this->ShowWindow(SW_SHOW);
	
	WCHAR pszTitle[64];
	StringCbPrintf(pszTitle, 64, L"远程SHELL    %s:%d\n", pClient->m_lpszIpAddress, pClient->m_wPort);
	this->SetWindowText(pszTitle);

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


//VOID CShellRemote::ShellOpen() {
//	//theApp.m_Server.SendPacket(pClient, SHELL_OPEN, NULL, 0);
//}


void CShellRemote::OnEnChangeEdit1()
{
	// TODO:  如果该控件是 RICHEDIT 控件，它将不
	// 发送此通知，除非重写 CDialogEx::OnInitDialog()
	// 函数并调用 CRichEditCtrl().SetEventMask()，
	// 同时将 ENM_CHANGE 标志“或”运算到掩码中。

	// TODO:  在此添加控件通知处理程序代码
}



//void CShellRemote::Start() {
//	this->Create(IDD_DIALOG2, GetDesktopWindow());
//	this->ShowWindow(SW_SHOW);
//}


// 执行命令
void CShellRemote::OnBnClickedButton1()
{
	WCHAR pbCommand[256];
	m_EditCommand.GetWindowText(pbCommand, 256);
	//MessageBox(pbCommand);
	DWORD dwCommandLength = (wcslen(pbCommand) + 1) * 2;
	theApp.m_Server.SendPacket(m_pSocketClient, SHELL_EXECUTE, (PBYTE)pbCommand, dwCommandLength);

	printf("IsConnected = %d\n", theApp.m_Server.m_pServer->IsConnected(m_pSocketClient->m_dwConnectId));
}





// 重写虚函数
void CShellRemote::OnRecvChildSocketClientPacket(CPacket* pPacket) {
	CClient* pClient = pPacket->m_pClient;
	
	switch (pPacket->m_PacketHead.wCommandId) {

		case SHELL_EXECUTE: {



			DWORD dwWideCharLength = MultiByteToWideChar(CP_ACP, 0, (CHAR*)pPacket->m_pbPacketBody, pPacket->m_dwPacketBodyLength, NULL, 0);
			WCHAR* pszWideCharTemp = new WCHAR[dwWideCharLength];
			MultiByteToWideChar(CP_ACP, 0, (CHAR*)pPacket->m_pbPacketBody, pPacket->m_dwPacketBodyLength, pszWideCharTemp, dwWideCharLength);
			
			if (dwWideCharLength > COMMAND_RESULT_BUFFER_LENGTH) {
				memcpy(m_pszWideChar, pszWideCharTemp + (dwWideCharLength - COMMAND_RESULT_BUFFER_LENGTH), COMMAND_RESULT_BUFFER_LENGTH*2);
				m_dwBufferTail = COMMAND_RESULT_BUFFER_LENGTH;
			}
			else if (m_dwBufferTail + dwWideCharLength <= COMMAND_RESULT_BUFFER_LENGTH){
				memcpy(m_pszWideChar + m_dwBufferTail, pszWideCharTemp, dwWideCharLength*2);
				m_dwBufferTail += dwWideCharLength;
			}
			else {
				memcpy(m_pszWideChar, m_pszWideChar + (m_dwBufferTail + dwWideCharLength - COMMAND_RESULT_BUFFER_LENGTH), (COMMAND_RESULT_BUFFER_LENGTH - dwWideCharLength)*2);
				memcpy(m_pszWideChar + (COMMAND_RESULT_BUFFER_LENGTH - dwWideCharLength), pszWideCharTemp, dwWideCharLength*2);
				m_dwBufferTail = COMMAND_RESULT_BUFFER_LENGTH;
			}
			m_pszWideChar[m_dwBufferTail] = 0;		// 字符串以0结尾

			delete[] pszWideCharTemp;
			pszWideCharTemp = nullptr;

			m_EditResult.SetWindowText(m_pszWideChar);
			m_EditResult.PostMessage(WM_VSCROLL, SB_BOTTOM, 0);
			break;
		}
		case SHELL_CLOSE:
			SendMessage(WM_CLOSE);
			break;
	}
}


void CShellRemote::OnOK()
{
	//什么也不写
}

//然后重载PreTranslateMessage函数  
//把ESC键的消息，用RETURN键的消息替换，这样，按ESC的时候，也会执行刚才的OnOK函数  
BOOL CShellRemote::PreTranslateMessage(MSG* pMsg)
{
	if (pMsg->message == WM_KEYDOWN && pMsg->wParam == VK_ESCAPE)
	{
		pMsg->wParam = VK_RETURN;   //将ESC键的消息替换为回车键的消息，这样，按ESC的时候  
									//也会去调用OnOK函数，而OnOK什么也不做，这样ESC也被屏蔽  
	}
	return   CDialog::PreTranslateMessage(pMsg);
}


void CShellRemote::OnClose() {

	DWORD dwConnectId = m_pSocketClient->m_dwConnectId;
	theApp.m_Server.m_pServer->Disconnect(dwConnectId);		// 断开这条子socket

	CDialogEx::OnClose();
}
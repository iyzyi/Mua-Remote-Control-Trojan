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
	theApp.m_Server.SendPacket(m_pClient, SHELL_EXECUTE, (PBYTE)pbCommand, dwCommandLength);

	printf("IsConnected = %d\n", theApp.m_Server.m_pServer->IsConnected(m_pClient->m_dwConnectId));
}





// 重写虚函数
void CShellRemote::OnRecvChildSocketClientPacket(CPacket* pPacket) {
	CClient* pClient = pPacket->m_pClient;
	
	switch (pPacket->m_PacketHead.wCommandId) {

	case SHELL_EXECUTE:
		MessageBox((WCHAR*)pPacket->m_pbPacketBody);

		//CString strText = _T("");
		////获得当前文本
		//m_EditResult.GetWindowText(strText);
		//strText += _T("ABC1");
		////设置追加后的文本
		//m_EditResult.SetWindowText();
		break;

	case SHELL_CLOSE:

		break;
	}
}
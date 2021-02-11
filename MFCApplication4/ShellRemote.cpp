// ShellRemote.cpp: 实现文件
//

#include "pch.h"
#include "ShellRemote.h"
#include "afxdialogex.h"
#include "resource.h"
#include "MFCApplication4.h"


// CShellRemote 对话框

IMPLEMENT_DYNAMIC(CShellRemote, CDialogEx)

CShellRemote::CShellRemote(CWnd* pParent /*=nullptr*/, CClient* pClient /*= nullptr*/)
	: CDialogEx(IDD_DIALOG2, pParent)
{

}

CShellRemote::~CShellRemote()
{
}

void CShellRemote::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CShellRemote, CDialogEx)
END_MESSAGE_MAP()


// CShellRemote 消息处理程序


VOID CShellRemote::ShellOpen() {
	//theApp.m_Server.SendPacket(pClient, SHELL_OPEN, NULL, 0);
}

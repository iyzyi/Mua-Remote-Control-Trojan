// RemoteShell.cpp: 实现文件
//

#include "pch.h"
#include "Resource.h"
#include "RemoteShell.h"
#include "afxdialogex.h"


// RemoteShell 对话框

IMPLEMENT_DYNAMIC(RemoteShell, CDialogEx)

RemoteShell::RemoteShell(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_DIALOG1, pParent)
{

}

RemoteShell::~RemoteShell()
{
}

void RemoteShell::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(RemoteShell, CDialogEx)
END_MESSAGE_MAP()


// RemoteShell 消息处理程序

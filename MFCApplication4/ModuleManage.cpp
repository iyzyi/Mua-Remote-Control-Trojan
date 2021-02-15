#include "pch.h"
#include "ModuleManage.h"
#include "ModuleShellRemote.h"
#include "MFCApplication4Dlg.h"


#include "Packet.h"

CModule::CModule(CClient* pSocketClient) {
	m_pSocketClient = pSocketClient;
	pSocketClient->m_pModule = this;
}


CModule::~CModule() {

}


// 虚函数
void CModule::OnRecvChildSocketClientPacket(CPacket* pPacket) {

}





void RunShellRemote(CClient* pClient) {
	CShellRemote* pDlg = new CShellRemote(nullptr, pClient);				// 创建对话框
	pClient->m_DialogInfo = { SHELL_REMOTE_DLG, pDlg->m_hWnd, pDlg };		// TODO 句柄不知道有木有问题，记得回来检查
}


void RunFileUpload(CClient* pClient) {
	//CShellRemote* pDlg = new CShellRemote(nullptr, pClient);
	//pClient->m_DialogInfo = { SHELL_REMOTE_DLG, pDlg->m_hWnd, pDlg };
	theApp.m_pMainWnd->PostMessage(WM_FILE_UPLOAD_CONNECT_SUCCESS, 0, NULL);
}







//#include "ModuleShellRemote.h"
//#include "resource.h"
//#include "MFCApplication4.h"
//#include "MFCApplication4Dlg.h"

//
//
//VOID OnRecvShellRemotePacket(CPacket* pPacket) {
//	CClient* pClient = pPacket->m_pClient;
//
//	PostMessage(theApp.m_pMainWnd->m_hWnd, WM_RECV_CHILD_SOCKET_CLIENT_PACKET, NULL, (LPARAM)pPacket->m_pClient);
//
//	//switch (pPacket->m_PacketHead.wCommandId) {
//
//	//case SHELL_CONNECT:
//	//	// 这里必须加大括号，不然C2360: pDlg的初始化操作由case标签跳过。我觉得这是作用域导致的。
//	//	// 解决方案：只需在 switch 语句之前定义它，或使用大括号{}以确保它在退出特定case之前超出了范围。
//	//	// All you need to do is either define it before the switch statement or use curly braces { } to make sure it goes out of scope before exiting a specific case.
//	//	// 现在不需要加大括号了，因为相关的代码已经转移至WM_RECV_SHELL_CONNECT_PACKET对应的事件处理函数中去了。
//	//{
//	//	// 向主界面传递消息
//	//	PostMessage(theApp.m_pMainWnd->m_hWnd, WM_RECV_SHELL_CONNECT_PACKET, NULL, (LPARAM)pPacket->m_pClient);
//	//	
//	//	// 涉及窗体的创建一定不要在这个函数内进行，此函数在HP-Socket的OnReceive()中被调用。
//	//	// 可以在事件处理函数中更新用户界面吗？答：不可以。事件处理函数由SocketIO线程触发，如果在事件
//	//	// 处理函数中更新用户界面会急剧降低应用程序性能并且很容易造成死锁，应该使用其他方法异步更新用户界面。
//
//	//	// 实践证明如果在这里初始化窗口，会导致窗口加载一半，一直转圈圈。
//	//}
//	//	break;
//
//	//case SHELL_EXECUTE:
//
//	//	break;
//
//	//case SHELL_CLOSE:
//
//	//	break;
//	//}
//
//
//}
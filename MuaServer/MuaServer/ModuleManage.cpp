#include "pch.h"
#include "ModuleManage.h"
#include "ModuleShellRemote.h"
#include "MuaServerDlg.h"


#include "Packet.h"

CModule::CModule(CSocketClient* pSocketClient) {
	// 如何一开始因为没有pSocketClient而将pSocketClient置nullptr，则等下需要手动调用InitModule完成传值
	if (pSocketClient != nullptr) {
		m_pSocketClient = pSocketClient;
		pSocketClient->m_pModule = this;
		m_pClient = m_pSocketClient->m_pClient;
	}
}


VOID CModule::InitModule(CSocketClient* pSocketClient) {
	ASSERT(pSocketClient);
	m_pSocketClient = pSocketClient;
	pSocketClient->m_pModule = this;
	m_pClient = m_pSocketClient->m_pClient;
}



CModule::~CModule() {

}


// 虚函数
void CModule::OnRecvChildSocketClientPacket(CPacket* pPacket) {

}





void RunShellRemote(CSocketClient* pSocketClient) {
	CShellRemote* pDlg = new CShellRemote(nullptr, pSocketClient);				// 创建对话框
	pSocketClient->m_DialogInfo = { SHELL_REMOTE_DLG, pDlg->m_hWnd, pDlg };		// TODO 句柄不知道有木有问题，记得回来检查
}


void RunFileUpload(CSocketClient* pSocketClient) {
}
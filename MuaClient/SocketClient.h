#pragma once
#include "pch.h"
#include "Crypto.h"
#include "ModuleManage.h"


class CModuleManage;			// 头文件相互包含，不提前声明这个类的和，会报错C3646: 未知重写说明符


class CSocketClient : public CTcpClientListener {

public:
	BOOL					m_bIsMainSocketClient;
	CSocketClient*			m_pMainSocketClient;

	CModuleManage*			m_pModuleManage;				// 组件管理对象，只有主socket可以创建
															// 原本想用m_ModuleManage而非m_pModuleManage的，但是编译死活通不过，
															// 两个头文件相互包含，递归来探寻需要分配的内存。但是指针就不需要递归探寻所需内存大小
															// 现在这个m_pModuleManage是new来的，记得delete

	CTcpPackClientPtr		m_pTcpPackClient;

	CCrypto					m_Crypto;

	CLIENT_STATUS			m_dwClientStatus;

	HANDLE					m_hChildSocketClientExitEvent;

public:

	CSocketClient(CSocketClient* pMainSocketClient = nullptr);
	~CSocketClient();

	BOOL StartSocketClient();

	BOOL SendPacket(COMMAND_ID dwCommandId, PBYTE pbPacketBody, DWORD dwPacketBodyLength);

	void WaitForExitEvent();			// 直到收到退出事件时，子socket才退出。


	// 重写回调函数
	virtual EnHandleResult OnPrepareConnect(ITcpClient* pSender, CONNID dwConnID, SOCKET socket);
	virtual EnHandleResult OnConnect(ITcpClient* pSender, CONNID dwConnID);
	virtual EnHandleResult OnHandShake(ITcpClient* pSender, CONNID dwConnID);
	virtual EnHandleResult OnSend(ITcpClient* pSender, CONNID dwConnID, const BYTE* pData, int iLength);
	virtual EnHandleResult OnReceive(ITcpClient* pSender, CONNID dwConnID, const BYTE* pData, int iLength);
	virtual EnHandleResult OnClose(ITcpClient* pSender, CONNID dwConnID, EnSocketOperation enOperation, int iErrorCode);

};
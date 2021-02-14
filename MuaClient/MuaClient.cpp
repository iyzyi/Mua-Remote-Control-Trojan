// MuaClient.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include "pch.h"
#include <iostream>
#include "SocketClient.h"
#include "Misc.h"
#include "Login.h"
#include "SocketClientManage.h"

void MainFunc(CSocketClient* pMainSocketClient);


int main()
{

	CSocketClient MainSocketClient;
	MainSocketClient.StartSocketClient();
	
	while (true) {
		MainFunc(&MainSocketClient);
	}
}


// SEH所在函数不能有对象展开，所以单独放到一个函数里
void MainFunc(CSocketClient* pMainSocketClient) {
	__try {
		if (!pMainSocketClient->m_pTcpPackClient->IsConnected()) {
			printf("正在重连服务端.....\n");
			pMainSocketClient->StartSocketClient();
		}
		Sleep(1000);			// 若未在线，则3秒重试一次。
	}
	__except (EXCEPTION_EXECUTE_HANDLER){
		pMainSocketClient->m_pTcpPackClient->Stop();
		MessageBox(0, L"无视异常，重连服务端", L"无视异常，重连服务端", 0);
		printf("无视异常，重连服务端\n");				// TODO：之后可以改成重启此程序
	}
}
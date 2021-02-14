// MuaClient.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include "pch.h"
#include <iostream>
#include "SocketClient.h"
#include "Misc.h"
#include "Login.h"
#include "SocketClientManage.h"
#include "MuaClient.h"


//CSocketClientManage* g_pSocketClientManage;			// 要放到全局里面定义，我之前一直放在函数里面。。。。就一直错

int main()
{
	//g_pSocketClientManage = new CSocketClientManage();

	CSocketClient MainSocketClient;
	MainSocketClient.StartSocketClient();

	//g_pSocketClientManage->AddNewSocketClientToList(&MainSocketClient);

	while (true) {
		if (!MainSocketClient.m_pTcpPackClient->IsConnected()) {
			printf("正在重连服务端.....\n");
			MainSocketClient.StartSocketClient();
		}
		Sleep(10000);			// 若未在线，则3秒重试一次。
	}
}
// TestRunMuaClientDll.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include <stdio.h>
#include <windows.h>

typedef void(*FUNC)();

int main()
{
	HINSTANCE  hDll = LoadLibrary(L"MuaClient.dll");
	FUNC pfnRunMuaClient = (FUNC)GetProcAddress(hDll, "RunMuaClient");
	pfnRunMuaClient();
	FreeLibrary(hDll);
}
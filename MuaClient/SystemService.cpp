#include "pch.h"
//#include "SystemService.h"
//
//
//void ShowError(WCHAR *lpszText)
//{
//	WCHAR szErr[MAX_PATH] = { 0 };
//	::wsprintf(szErr, L"%s Error!\nError Code Is:%d\n", lpszText, ::GetLastError());
//	::MessageBox(NULL, szErr, L"ERROR", MB_OK | MB_ICONERROR);
//}
//
//
//// 0 加载服务    1 启动服务    2 停止服务    3 删除服务
//BOOL SystemServiceOperate(WCHAR *lpszDriverPath, int iOperateType)
//{
//	BOOL bRet = TRUE;
//	WCHAR szName[MAX_PATH] = { 0 };
//
//	::lstrcpy(szName, lpszDriverPath);
//	// 过滤掉文件目录，获取文件名
//	::PathStripPath(szName);
//
//	SC_HANDLE shOSCM = NULL, shCS = NULL;
//	SERVICE_STATUS ss;
//	DWORD dwErrorCode = 0;
//	BOOL bSuccess = FALSE;
//	// 打开服务控制管理器数据库
//	shOSCM = ::OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
//	if (!shOSCM)
//	{
//		ShowError((LPWSTR)L"OpenSCManager");
//		return FALSE;
//	}
//
//	if (0 != iOperateType)
//	{
//		// 打开一个已经存在的服务
//		shCS = OpenService(shOSCM, szName, SERVICE_ALL_ACCESS);
//		if (!shCS)
//		{
//			ShowError((LPWSTR)L"OpenService");
//			::CloseServiceHandle(shOSCM);
//			shOSCM = NULL;
//			return FALSE;
//		}
//	}
//
//	switch (iOperateType)
//	{
//	case 0:
//	{
//		// 创建服务
//		// SERVICE_AUTO_START   随系统自动启动
//		// SERVICE_DEMAND_START 手动启动
//		shCS = ::CreateService(shOSCM, szName, szName,
//			SERVICE_ALL_ACCESS,
//			SERVICE_WIN32_OWN_PROCESS | SERVICE_INTERACTIVE_PROCESS,
//			SERVICE_AUTO_START,
//			SERVICE_ERROR_NORMAL,
//			lpszDriverPath, NULL, NULL, NULL, NULL, NULL);
//		if (!shCS)
//		{
//			ShowError((LPWSTR)L"CreateService");
//			bRet = FALSE;
//		}
//		break;
//	}
//	case 1:
//	{
//		// 启动服务
//		if (!::StartService(shCS, 0, NULL))
//		{
//			ShowError((LPWSTR)L"StartService");
//			bRet = FALSE;
//		}
//		break;
//	}
//	case 2:
//	{
//		// 停止服务
//		if (!::ControlService(shCS, SERVICE_CONTROL_STOP, &ss))
//		{
//			ShowError((LPWSTR)L"ControlService");
//			bRet = FALSE;
//		}
//		break;
//	}
//	case 3:
//	{
//		// 删除服务
//		if (!::DeleteService(shCS))
//		{
//			ShowError((LPWSTR)L"DeleteService");
//			bRet = FALSE;
//		}
//		break;
//	}
//	default:
//		break;
//	}
//	// 关闭句柄
//	if (shCS)
//	{
//		::CloseServiceHandle(shCS);
//		shCS = NULL;
//	}
//	if (shOSCM)
//	{
//		::CloseServiceHandle(shOSCM);
//		shOSCM = NULL;
//	}
//
//	return bRet;
//}
//
//
//
//
//// 全局变量
//char g_szServiceName[MAX_PATH] = "Windows Defender自动更新";    // 服务名称 
//SERVICE_STATUS_HANDLE g_ServiceStatusHandle = { 0 };
//
//
//int _tmain(int argc, _TCHAR* argv[])
//{
//	// 注册服务入口函数
//	SERVICE_TABLE_ENTRY stDispatchTable[] = { { g_szServiceName, (LPSERVICE_MAIN_FUNCTION)ServiceMain }, { NULL, NULL } };
//	::StartServiceCtrlDispatcher(stDispatchTable);
//
//	return 0;
//}
//
//
//void __stdcall ServiceMain(DWORD dwArgc, char *lpszArgv)
//{
//	//g_ServiceStatusHandle = RegisterServiceCtrlHandler(g_szServiceName, ServiceCtrlHandle);
//
//	TellSCM(SERVICE_START_PENDING, 0, 1);
//	TellSCM(SERVICE_RUNNING, 0, 0);
//
//	MyTask();
//
//	while (TRUE)
//	{
//		Sleep(5000);
//	}
//}
//
//
//void __stdcall ServiceCtrlHandle(DWORD dwOperateCode)
//{
//	switch (dwOperateCode)
//	{
//	case SERVICE_CONTROL_PAUSE:
//	{
//		// 暂停
//		TellSCM(SERVICE_PAUSE_PENDING, 0, 1);
//		TellSCM(SERVICE_PAUSED, 0, 0);
//		break;
//	}
//	case SERVICE_CONTROL_CONTINUE:
//	{
//		// 继续
//		TellSCM(SERVICE_CONTINUE_PENDING, 0, 1);
//		TellSCM(SERVICE_RUNNING, 0, 0);
//		break;
//	}
//	case SERVICE_CONTROL_STOP:
//	{
//		// 停止
//		TellSCM(SERVICE_STOP_PENDING, 0, 1);
//		TellSCM(SERVICE_STOPPED, 0, 0);
//		break;
//	}
//	case SERVICE_CONTROL_INTERROGATE:
//	{
//		// 询问
//		break;
//	}
//	default:
//		break;
//	}
//}
//
//BOOL TellSCM(DWORD dwState, DWORD dwExitCode, DWORD dwProgress)
//{
//	SERVICE_STATUS serviceStatus = { 0 };
//	BOOL bRet = FALSE;
//
//	RtlZeroMemory(&serviceStatus, sizeof(serviceStatus));
//	serviceStatus.dwServiceType = SERVICE_WIN32_OWN_PROCESS;
//	serviceStatus.dwCurrentState = dwState;
//	serviceStatus.dwControlsAccepted = SERVICE_ACCEPT_STOP | SERVICE_ACCEPT_PAUSE_CONTINUE | SERVICE_ACCEPT_SHUTDOWN;
//	serviceStatus.dwWin32ExitCode = dwExitCode;
//	serviceStatus.dwWaitHint = 3000;
//
//	bRet = SetServiceStatus(g_ServiceStatusHandle, &serviceStatus);
//	return bRet;
//}
//
//void DoTask()
//{
//	// 自己程序实现部分代码放在这里
//}
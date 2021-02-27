#pragma once


#include <Windows.h>
#include <Shlwapi.h>
#pragma comment(lib, "Shlwapi.lib")


// 0 加载服务    1 启动服务    2 停止服务    3 删除服务
BOOL SystemServiceOperate(char *lpszDriverPath, int iOperateType);


// 服务入口函数以及处理回调函数
extern "C" __declspec(dllexport) void __stdcall ServiceMain(DWORD dwArgc, char *lpszArgv);
void __stdcall ServiceCtrlHandle(DWORD dwOperateCode);
BOOL TellSCM(DWORD dwState, DWORD dwExitCode, DWORD dwProgress);
void MyTask();
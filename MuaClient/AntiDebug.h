#pragma once

#include "pch.h"
#include <windows.h>
#include <process.h>
#include <TlHelp32.h>
#include <winternl.h>
#pragma comment(lib,"ntdll.lib")


bool CheckDebug_INT_2D() {
	BOOL bDebugging = FALSE;
	__asm {
		// install SEH
		push handler
		push DWORD ptr fs : [0]
		mov DWORD ptr fs : [0], esp
		// OD会忽略0x2d和nop，继续向后执行
		// 这时候可以选择只是检测调试器还是跑飞
		int 0x2d
		nop
		mov bDebugging, 1
		jmp normal_code
		handler :
		mov eax, dword ptr ss : [esp + 0xc]
			mov dword ptr ds : [eax + 0xb8], offset normal_code
			mov bDebugging, 0
			xor eax, eax
			retn
			normal_code :
		//   remove SEH
		pop dword ptr fs : [0]
			add esp, 4
	}
	return bDebugging;
}


bool CheckDebug_EnumProcess() {
	PROCESSENTRY32 pe32 = { sizeof(pe32) };
	HANDLE hProcessSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	if (hProcessSnap == INVALID_HANDLE_VALUE)
	{
		return FALSE;
	}
	Process32First(hProcessSnap, &pe32);
	do
	{
		// 这里只比较了OllyDbg,也可以添加其他的调试分析工具名
		if (_tcsicmp(pe32.szExeFile, TEXT("OllyDbg.exe")) == 0)
		{
			CloseHandle(hProcessSnap);
			return TRUE;
		}

		if (_tcsicmp(pe32.szExeFile, TEXT("x32dbg.exe")) == 0)
		{
			CloseHandle(hProcessSnap);
			return TRUE;
		}

		if (_tcsicmp(pe32.szExeFile, TEXT("x64dbg.exe")) == 0)
		{
			CloseHandle(hProcessSnap);
			return TRUE;
		}

		if (_tcsicmp(pe32.szExeFile, TEXT("ida.exe")) == 0)
		{
			CloseHandle(hProcessSnap);
			return TRUE;
		}

		if (_tcsicmp(pe32.szExeFile, TEXT("ida64.exe")) == 0)
		{
			CloseHandle(hProcessSnap);
			return TRUE;
		}
	} while (Process32Next(hProcessSnap, &pe32));
	CloseHandle(hProcessSnap);
	return FALSE;
}
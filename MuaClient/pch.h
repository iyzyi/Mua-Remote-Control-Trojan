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



enum COMMAND_ID {
	CRYPTO_KEY,				// 传递通信密钥和IV

	LOGIN,					// 上线包

	ECHO,					// 测试

	SHELL_REMOTE,			// 远程SHELL
	FILE_TRANSFOR,			// 文件传输
	SCREEN_MONITOR,			// 屏幕监控
	MESSAGE_BOX,			// 弹窗
};




#endif //PCH_H

#pragma once

#include "pch.h"


enum CLIENT_STATUS {
	NOT_ONLINE,			// 客户端不在线
	LOGINING,			// 传递对称密钥的过程。这阶段的包是明文的（后续可能改成RSA加密）
	LOGINED				// 已登录，指密钥传递完成后，正式建立通信。
};


typedef struct _CLIENT_CONTEXT {
	CONNID					dwConnextId;

	PBYTE					pbCryptoKey;
	PBYTE					pbCryptoIv;

	CLIENT_STATUS			dwClientStatus;

}CLIENT_CONTEXT, *CLIENT_CONTEXT;

class CClient {
public:
	CClient();

	~CClient();
};
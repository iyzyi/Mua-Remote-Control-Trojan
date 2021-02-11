#pragma once

#include "pch.h"
#include "Crypto.h"


enum CLIENT_STATUS {
	NOT_ONLINE,			// 客户端（即受控端）不在线
	//HAVE_CRYPTO_KEY,	// 主控端接收到了被控端发来的对称密钥。这阶段的包是明文的（后续可能改成RSA加密）。
	WAIT_FOR_LOGIN,		// 等待上线包（使用对称加密算法加密），该包有IP，CPU，系统版本等信息。
	LOGINED				// 已登录，（接收到通信密钥和上线包后）正式建立通信。
};





// 单个客户端
class CClient {
public:
	CONNID					m_dwConnectId;
	CLIENT_STATUS			m_dwClientStatus;
	CCrypto					m_Crypto;

	WCHAR					m_lpszIpAddress[20];
	WORD					m_wPort;
	
	BOOL					m_bIsMainSocketServer;



	// 构成双向链表，方便CClientManage管理
	// 链表中以ConnectId做唯一标识
	CClient*				m_pLastClient;
	CClient*				m_pNextClient;

	
public:
	CClient(CONNID dwConnectId, LPWSTR lpszAddress, WORD usPort, BOOL isMainSocketServer);
	CClient();
	~CClient();

	VOID SetCryptoKey(PBYTE pbCryptoKey = NULL, PBYTE pbCryptoIv = NULL);

};



// 管理所有客户端
class CClientManage {
public:
	CClientManage();
	~CClientManage();


	// 双向链表
	VOID AddNewClientToList(CClient *pClient);

	// 返回是否删除成功（删除失败主要原因可能是链表中并没有这个Client）
	BOOL DeleteClientFromList(CONNID dwConnectId);
	VOID DeleteClientFromList(CClient *pClient);

	VOID DeleteAllClientFromList();

	// Client是否存在于链表中，存在则返回Client地址，不存在返回NULL
	CClient* SearchClient(CONNID dwConnectId);


public:

	// Client链表的第一个结点，这个结点的所有数据都是空的，
	// 仅用来索引链表的头部，只有m_pNextClient这个数据是非空的。
	// m_pClientListHead->m_pNextClient存放真正的第一个Client的地址
	CClient *m_pClientListHead;
	
	// 该结点指向链表最后一个结点，这个结点是有数据的结点
	//（除非链表为空，此时m_pClientListHead=m_pClientListTail）
	CClient *m_pClientListTail;

private:
	CRITICAL_SECTION m_Lock;					// 链表操作的锁

	DWORD m_dwClientNum;
	DWORD m_dwMainSocketClientNum;
	DWORD m_dwChildSocketClientNum;
};
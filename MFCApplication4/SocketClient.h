#pragma once

#include "SocketServer.h"

class CModule;


enum SOCKET_CLIENT_STATUS {
	NOT_ONLINE,			// 客户端（即受控端）不在线
	WAIT_FOR_LOGIN,		// 等待上线包（使用对称加密算法加密），该包有IP，CPU，系统版本等信息。
	LOGINED				// 已登录，（接收到通信密钥和上线包后）正式建立通信。
};



// 一个CSocketClient类对象表示一条socket连接（一个CClient有多个socket连接）

class CSocketClient {
public:
	CClient*				m_pClient;				// 所属的客户端

	CONNID					m_dwConnectId;

	SOCKET_CLIENT_STATUS	m_dwSocketClientStatus;

	CCrypto					m_Crypto;

	WCHAR					m_lpszIpAddress[IP_ADDRESS_MAX_LENGTH];
	WORD					m_wPort;

	BOOL					m_bIsMainSocketServer;	// 是否是主socket

	DIALOG_INFO				m_DialogInfo;			// 仅针对子socket。一个子socket最多可以有一个对应的窗口，如远程shell窗口
	CModule*				m_pModule;				// 针对子socket


	// 构成双向链表，方便CClient管理
	// 链表中以ConnectId做唯一标识
	// 只放子socket
	CSocketClient*				m_pLastChildSocketClient;
	CSocketClient*				m_pNextChildSocketClient;


public:
	CSocketClient(CONNID dwConnectId, BOOL bIsMainSocketClient, CModule* pModule = nullptr);
	CSocketClient();
	~CSocketClient();

	BOOL SetCryptoKey(PBYTE pbRsaEncrypted);
};




class CClient {
public:
	CClient(CSocketClient* pSocketClient);
	CClient();
	~CClient();

	VOID ChangeNoChildSocketClientEvent();
	VOID WaitForNoChildSocketClientEvent();

	// 断开该client的全部子socket的连接
	VOID DisConnectedAllChildSocketClient();

	VOID AddNewChildSocketClientToList(CSocketClient *pSocketClient);
	BOOL DeleteChildSocketClientFromList(CONNID dwConnectId);
	VOID DeleteChildSocketClientFromList(CSocketClient *pSocketClient);
	VOID DeleteAllChildSocketClientFromList();
	CSocketClient* SearchChildSocketClient(CONNID dwConnectId);
	//VOID DeleteAllChildClientByOneIP(CSocketClient *pSocketClient);

public:
	CSocketClient*			m_pMainSocketClient;

	CSocketClient*			m_pChildSocketClientListHead;		// 链表内只放子socket
	CSocketClient*			m_pChildSocketClientListTail;
	DWORD					m_dwChildSocketClientNum;

	WCHAR					m_lpszIpAddress[IP_ADDRESS_MAX_LENGTH];
	
public:
	// 构成双向链表，方便CClientManage管理
	CClient*				m_pLastClient;
	CClient*				m_pNextClient;

public:
	HANDLE					m_FileUploadConnectSuccessEvent;	// 收到客户端发来的FILL_UPLOAD_CONNECT包时触发此事件
	HANDLE					m_FileDownloadConnectSuccessEvent;



	// 暂存某一次收到CONNECT包时的CSocketClient
	CSocketClient*			m_pFileUploadConnectSocketClientTemp;
	CSocketClient*			m_pFileDownloadConnectSocketClientTemp;


private:
	CRITICAL_SECTION		m_Lock;								// 链表操作的锁
	HANDLE					m_hNoChildSocketClientEvent;		// 没有子socket时触发本事件
};
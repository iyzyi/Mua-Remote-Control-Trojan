#include "pch.h"
#include "SocketClient.h"
#include "MuaServer.h"




// 以下是CSocketClient的命名空间，一个CSocketClient类对象表示一条socket连接（一个CClient有多个socket连接）

CSocketClient::CSocketClient(CONNID dwConnectId, BOOL bIsMainSocketClient, CModule* pModule) {
	m_dwConnectId = dwConnectId;

	m_bIsMainSocketServer = bIsMainSocketClient;

	// 如果不是主socket，则通过比较IP来获取该子socket所属的客户端（主socket在创建的时候手动设置所属的客户端）
	if (!m_bIsMainSocketServer) {
		m_pClient = theApp.m_Server.m_pClientManage->SearchClientByIp(m_dwConnectId);
		ASSERT(m_pClient != nullptr);
	}	

	
	// 通过ConnectId获取IP地址和端口
	int dwIpAddressLength = IP_ADDRESS_MAX_LENGTH;
	theApp.m_Server.m_pTcpPackServer->GetRemoteAddress(m_dwConnectId, m_lpszIpAddress, dwIpAddressLength, m_wPort);

	// 被控端发来的第一个包是传递密钥的封包，接收密钥后，状态就是等待上线包
	m_dwSocketClientStatus = WAIT_FOR_LOGIN;

	m_pLastChildSocketClient = nullptr;
	m_pNextChildSocketClient = nullptr;
}


CSocketClient::CSocketClient() {
	m_dwConnectId					= 0;
	ZeroMemory(&m_lpszIpAddress, sizeof(m_lpszIpAddress));
	m_wPort							= 0;
	m_bIsMainSocketServer			= false;
	m_dwSocketClientStatus			= NOT_ONLINE;
	m_pLastChildSocketClient				= nullptr;
	m_pNextChildSocketClient				= nullptr;
}


CSocketClient::~CSocketClient() {
	if (theApp.m_Server.m_pTcpPackServer->IsConnected(m_dwConnectId)) {
		theApp.m_Server.m_pTcpPackServer->Disconnect(m_dwConnectId, true);
	}
}


// 设置密钥（key和iv）
BOOL CSocketClient::SetCryptoKey(PBYTE pbRsaEncrypted) {
	m_Crypto = CCrypto(AES_128_CFB, pbRsaEncrypted);
	return m_Crypto.m_bInitSuccess;
}









// 以下是CClient的命名空间，一个CClient有多个socket连接，每个socket连接对应一个CSocketClient

CClient::CClient(CSocketClient* pSocketClient) {
	m_pMainSocketClient				= pSocketClient;

	// 第一个结点是空的，用来索引
	m_pChildSocketClientListHead	= new CSocketClient();;
	m_pChildSocketClientListTail	= m_pChildSocketClientListHead;

	m_dwChildSocketClientNum		= 0;

	wcscpy_s(m_lpszIpAddress, pSocketClient->m_lpszIpAddress);

	InitializeCriticalSection(&m_Lock);

	// 第二个参数表示手动重置事件, 第三个参数设置初始状态为信号状态
	m_hNoChildSocketClientEvent = CreateEvent(NULL, true, true, NULL);
	//SetEvent(m_hNoChildSocketClientEvent);			// 设为信号状态

	// 自动重置事件，初始状态无信号
	m_FileUploadConnectSuccessEvent = CreateEvent(NULL, false, false, NULL);
	m_FileDownloadConnectSuccessEvent = CreateEvent(NULL, false, false, NULL);
}


CClient::CClient() {
	m_pMainSocketClient = nullptr;

	// 第一个结点是空的，用来索引
	m_pChildSocketClientListHead = new CSocketClient();;
	m_pChildSocketClientListTail = m_pChildSocketClientListHead;

	m_dwChildSocketClientNum = 0;

	InitializeCriticalSection(&m_Lock);

	m_hNoChildSocketClientEvent = nullptr;

	m_FileUploadConnectSuccessEvent = nullptr;
	m_FileDownloadConnectSuccessEvent = nullptr;
}


CClient::~CClient() {
	DeleteCriticalSection(&m_Lock);

	if (m_pMainSocketClient != nullptr) {
		delete m_pMainSocketClient;
		m_pMainSocketClient = nullptr;
	}
	
	if (m_pChildSocketClientListHead != nullptr) {
		delete m_pChildSocketClientListHead;
		m_pChildSocketClientListHead = nullptr;
	}

	if (m_hNoChildSocketClientEvent != nullptr) {
		CloseHandle(m_hNoChildSocketClientEvent);
		m_hNoChildSocketClientEvent = nullptr;
	}

	if (m_FileUploadConnectSuccessEvent != nullptr) {
		CloseHandle(m_FileUploadConnectSuccessEvent);
		m_FileUploadConnectSuccessEvent = nullptr;
	}

	if (m_FileDownloadConnectSuccessEvent != nullptr) {
		CloseHandle(m_FileDownloadConnectSuccessEvent);
		m_FileDownloadConnectSuccessEvent = nullptr;
	}
}


VOID CClient::ChangeNoChildSocketClientEvent() {
	if (m_dwChildSocketClientNum == 0) {
		SetEvent(m_hNoChildSocketClientEvent);			// 设为信号状态
	}
	else {
		ResetEvent(m_hNoChildSocketClientEvent);		// 设为非信号状态
	}
}


VOID CClient::WaitForNoChildSocketClientEvent() {
	WaitForSingleObject(m_hNoChildSocketClientEvent, INFINITE);
}


// 断开该client的全部子socket的连接
VOID CClient::DisConnectedAllChildSocketClient() {
	__try {
		EnterCriticalSection(&m_Lock);

		CSocketClient *pSocketClientNode = m_pChildSocketClientListHead;
		while (pSocketClientNode->m_pNextChildSocketClient != nullptr) {
			DWORD dwConnectId = pSocketClientNode->m_pNextChildSocketClient->m_dwConnectId;
			// 只是disconnect链接，没有析构哦
			theApp.m_Server.m_pTcpPackServer->Disconnect(dwConnectId, true);

			pSocketClientNode = pSocketClientNode->m_pNextChildSocketClient;
		}
	}
	__finally {
		LeaveCriticalSection(&m_Lock);
	}
}


// 双向链表，新结点插在列表尾部
VOID CClient::AddNewChildSocketClientToList(CSocketClient *pSocketClient) {
	ASSERT(m_pMainSocketClient != nullptr);

	__try {
		EnterCriticalSection(&m_Lock);

		m_pChildSocketClientListTail->m_pNextChildSocketClient = pSocketClient;		// 链表为空时m_pChildSocketClientListTail=m_pChildSocketClientListHead
		pSocketClient->m_pLastChildSocketClient = m_pChildSocketClientListTail;
		m_pChildSocketClientListTail = pSocketClient;

		m_dwChildSocketClientNum++;

#ifdef _DEBUG
		USES_CONVERSION;
		DebugPrint("[子socket上线 - %s:%d] 本IP共%d条子socket连接\n",
			W2A(pSocketClient->m_lpszIpAddress), pSocketClient->m_wPort, m_dwChildSocketClientNum);
#endif

		ChangeNoChildSocketClientEvent();
	}
	__finally {
		LeaveCriticalSection(&m_Lock);
	}
}


// 删除SocketClient, 返回是否删除成功（删除失败主要原因可能是链表中并没有这个SocketClient）
BOOL CClient::DeleteChildSocketClientFromList(CONNID dwConnectId) {
	ASSERT(m_pMainSocketClient != nullptr);

	CSocketClient *pSocketClient = SearchChildSocketClient(dwConnectId);
	if (pSocketClient == nullptr) {
		return false;
	}
	else {
		DeleteChildSocketClientFromList(pSocketClient);
		return true;
	}
}


// 从链表中删除一个节点
VOID CClient::DeleteChildSocketClientFromList(CSocketClient *pSocketClient) {
	ASSERT(m_pMainSocketClient != nullptr);

	__try {
		EnterCriticalSection(&m_Lock);

		if (pSocketClient == m_pChildSocketClientListTail) {			// 要删除的结点是最后一个结点
			pSocketClient->m_pLastChildSocketClient->m_pNextChildSocketClient = nullptr;
			m_pChildSocketClientListTail = pSocketClient->m_pLastChildSocketClient;
			pSocketClient->m_pLastChildSocketClient = nullptr;
		}
		else {
			pSocketClient->m_pLastChildSocketClient->m_pNextChildSocketClient = pSocketClient->m_pNextChildSocketClient;
			pSocketClient->m_pNextChildSocketClient->m_pLastChildSocketClient = pSocketClient->m_pLastChildSocketClient;
			pSocketClient->m_pLastChildSocketClient = nullptr;
			pSocketClient->m_pNextChildSocketClient = nullptr;
		}

		m_dwChildSocketClientNum--;

#ifdef _DEBUG
		USES_CONVERSION;			// 使用A2W之前先声明这个
		DebugPrint("[子socket下线 - %s:%d] 本IP共%d条子socket连接\n",
			W2A(pSocketClient->m_lpszIpAddress), pSocketClient->m_wPort, m_dwChildSocketClientNum);
#endif

		delete pSocketClient;				// SocketClient在这里释放内存

		ChangeNoChildSocketClientEvent();
	}
	__finally {
		LeaveCriticalSection(&m_Lock);
	}
}


// 从链表中删除所有的节点
VOID CClient::DeleteAllChildSocketClientFromList() {
	ASSERT(m_pMainSocketClient != nullptr);

	// 从链表尾部开始删
	while (m_pChildSocketClientListTail != m_pChildSocketClientListHead) {
		DeleteChildSocketClientFromList(m_pChildSocketClientListTail);
	}
}


// SocketClient是否存在于链表中，存在则返回SocketClient地址，不存在返回nullptr
CSocketClient* CClient::SearchChildSocketClient(CONNID dwConnectId) {
	ASSERT(m_pMainSocketClient != nullptr);
	
	CSocketClient* Ret = nullptr;

	__try {
		EnterCriticalSection(&m_Lock);

		CSocketClient *pSocketClientNode = m_pChildSocketClientListHead;
		while (pSocketClientNode->m_pNextChildSocketClient != nullptr) {
			if (pSocketClientNode->m_pNextChildSocketClient->m_dwConnectId == dwConnectId) {
				Ret = pSocketClientNode->m_pNextChildSocketClient;
				__leave;			// 原本这里直接返回，没有释放锁。。。
			}
			pSocketClientNode = pSocketClientNode->m_pNextChildSocketClient;
		}

	}
	__finally {
		LeaveCriticalSection(&m_Lock);
	}
	
	return Ret;
}
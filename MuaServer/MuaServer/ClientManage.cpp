#include "pch.h"
#include "ClientManage.h"
#include "Packet.h"
#include "ModuleManage.h"
#include "MuaServer.h"
#include "SocketClient.h"




// 以下是CClientManage的命名空间
CClientManage::CClientManage() {

	// 初始化链表，链表头结点是空结点。

	m_pClientListHead = new CClient();
	m_pClientListTail = m_pClientListHead;
	m_dwClientNum = 0;
	InitializeCriticalSection(&m_ClientLock);

}


CClientManage::~CClientManage() {
	DeleteCriticalSection(&m_ClientLock);

	if (m_pClientListHead != nullptr) {
		delete m_pClientListHead;
		m_pClientListHead = nullptr;
	}

}





// 以下函数为操作 存放CClient的链表 的函数

// 双向链表，新结点插在列表尾部
VOID CClientManage::AddNewClientToList(CClient *pClient) {
	EnterCriticalSection(&m_ClientLock);

	m_pClientListTail->m_pNextClient = pClient;		// 链表为空时m_pClientListTail=m_pClientListHead
	pClient->m_pLastClient = m_pClientListTail;
	m_pClientListTail = pClient;

	m_dwClientNum++;
		
#ifdef _DEBUG
	USES_CONVERSION;
	DebugPrint("[主socket上线 - %s:%d] 当前共有%d个客户端(主socket)在线\n",
		W2A(pClient->m_pMainSocketClient->m_lpszIpAddress), pClient->m_pMainSocketClient->m_wPort, m_dwClientNum);
#endif

	LeaveCriticalSection(&m_ClientLock);
}


// 删除Client, 返回是否删除成功（删除失败主要原因可能是链表中并没有这个Client）
BOOL CClientManage::DeleteClientFromList(CONNID dwConnectId) {
	CClient *pClient = SearchClient(dwConnectId);
	if (pClient == NULL) {
		return false;
	} else {
		DeleteClientFromList(pClient);
		return true;
	}
}


VOID CClientManage::DeleteClientFromList(CClient *pClient) {

	if (pClient == m_pClientListTail) {			// 要删除的结点是最后一个结点
		pClient->m_pLastClient->m_pNextClient = NULL;
		m_pClientListTail = pClient->m_pLastClient;
		pClient->m_pLastClient = NULL;
	}
	else {
		pClient->m_pLastClient->m_pNextClient = pClient->m_pNextClient;
		pClient->m_pNextClient->m_pLastClient = pClient->m_pLastClient;
		pClient->m_pLastClient = NULL;
		pClient->m_pNextClient = NULL;
	}

	m_dwClientNum--;

#ifdef _DEBUG
	USES_CONVERSION;			// 使用A2W之前先声明这个
	DebugPrint("[主socket下线 - %s:%d] 当前共有%d个客户端(主socket)在线\n",
		W2A(pClient->m_pMainSocketClient->m_lpszIpAddress), pClient->m_pMainSocketClient->m_wPort, m_dwClientNum);
#endif

	// 使该主socket下线
	delete pClient;
}


VOID CClientManage::DeleteAllClientFromList() {
	// 从链表尾部开始删
	while (m_pClientListTail != m_pClientListHead) {
		DeleteClientFromList(m_pClientListTail);
	}
}


// Client是否存在于链表中，存在则返回Client地址，不存在返回nullptr
CClient* CClientManage::SearchClient(CONNID dwConnectId) {

	CClient* Ret = nullptr;

	__try {
		EnterCriticalSection(&m_ClientLock);

		CClient *pClientNode = m_pClientListHead;
		while (pClientNode->m_pNextClient != nullptr) {
			if (pClientNode->m_pNextClient->m_pMainSocketClient->m_dwConnectId == dwConnectId) {
				Ret = pClientNode->m_pNextClient;
				__leave;
			}
			pClientNode = pClientNode->m_pNextClient;
		}

	}
	__finally {
		LeaveCriticalSection(&m_ClientLock);
	}

	return Ret;
}


// CSocketClient是否存在，存在则返回其地址，不存在返回nullptr
// 注意，这个查找的是CSocketClient，原理是遍历每个CClient中的存放子SocketClient的链表
// 和遍历每个CClient的主SocketClient
CSocketClient* CClientManage::SearchSocketClient(CONNID dwConnectId) {
	CClient *pClientNode = m_pClientListHead;
	while (pClientNode->m_pNextClient != nullptr) {

		// 该客户端的主socket
		if (pClientNode->m_pNextClient->m_pMainSocketClient->m_dwConnectId == dwConnectId) {
			return pClientNode->m_pNextClient->m_pMainSocketClient;
		}

		// 遍历该客户端的子socket
		CSocketClient* pSocketClientTemp = pClientNode->m_pNextClient->SearchChildSocketClient(dwConnectId);
		if (pSocketClientTemp != nullptr) {
			return pSocketClientTemp;
		}

		pClientNode = pClientNode->m_pNextClient;
	}

	return nullptr;
}


// 我们假定一台机子只运行一个Mua客户端（被控端），所以一个IP就是一个客户端。
// 子socket属于哪个客户端，完全就是靠IP来分辨的
CClient* CClientManage::SearchClientByIp(CONNID dwConnectId) {
	
	CClient* Ret = nullptr;

	__try {
		EnterCriticalSection(&m_ClientLock);

		// 获取该dwConnectId所属的IP
		WCHAR lpszIpAddress[IP_ADDRESS_MAX_LENGTH];
		int dwIpAddressLength = IP_ADDRESS_MAX_LENGTH;
		USHORT wPort = 0;
		theApp.m_Server.m_pTcpPackServer->GetRemoteAddress(dwConnectId, lpszIpAddress, dwIpAddressLength, wPort);

		CClient *pClientNode = m_pClientListHead;
		while (pClientNode->m_pNextClient != nullptr) {
			// 比较IP是否相同
			DWORD bRet = wcscmp(lpszIpAddress, pClientNode->m_pNextClient->m_pMainSocketClient->m_lpszIpAddress);
			if (bRet == 0) {
				Ret = pClientNode->m_pNextClient;
				__leave;
			}
			pClientNode = pClientNode->m_pNextClient;
		}
	}
	__finally {
		LeaveCriticalSection(&m_ClientLock);
	}
	
	return Ret;
}
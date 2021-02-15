#include "pch.h"
#include "ClientManage.h"
#include "Packet.h"
#include "ModuleManage.h"
#include "MFCApplication4.h"
#include "SocketClient.h"




// 以下是CClientManage的命名空间
CClientManage::CClientManage() {

	// 初始化链表，链表头结点是空结点。

	//m_pClientListHead = &CClient();
	// 上面的写法申请的是局部变量，这个类对象在本函数结束时就被析构了，坑死了我。
	// 析构完了之后，m_pClientListHead执向的地址不再是类对象，后续的逻辑运行都是不可预料的了。
	m_pClientListHead = new CClient();
	m_pClientListTail = m_pClientListHead;
	m_dwClientNum = 0;
	InitializeCriticalSection(&m_ClientLock);



	//m_pChildSocketClientListHead = new CSocketClient();
	//m_pChildSocketClientListTail = m_pChildSocketClientListHead;
	//m_dwChildSocketClientNum = 0;
	//InitializeCriticalSection(&m_ChildSocketClientLock);
}


CClientManage::~CClientManage() {
	DeleteCriticalSection(&m_ClientLock);
	//DeleteCriticalSection(&m_ChildSocketClientLock);

	if (m_pClientListHead != nullptr) {
		delete m_pClientListHead;
		m_pClientListHead = nullptr;
	}

	//if (m_pChildSocketClientListHead != nullptr) {
	//	delete m_pChildSocketClientListHead;
	//	m_pChildSocketClientListHead = nullptr;
	//}
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
	printf("[主socket上线 - %s:%d] 当前共有%d个客户端(主socket)在线\n",
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
	printf("[主socket下线 - %s:%d] 当前共有%d个客户端(主socket)在线\n",
		W2A(pClient->m_pMainSocketClient->m_lpszIpAddress), pClient->m_pMainSocketClient->m_wPort, m_dwClientNum);
#endif

	// 使该主socket所拥有的全部子socket下线
	//pClient->DeleteAllSocketClientFromList();
	

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
		WCHAR lpszIpAddress[20];
		int dwIpAddressLength = 20;
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





//
//// 删除一个主socket对应的全部子socket
//// 在Client链表中搜索与之相同IP的其他client, 如果不是主socket，那么就认定为是这个主socket的子socket, 一同断开连接
//// 这里假定一个IP只上线一个主socket, 不然其他的子socket实在无法区分所属的主socket.
//VOID CClientManage::DeleteAllChildClientByOneIP(CClient *pClient) {
//	CClient *pClientNode = m_pClientListHead;
//	while (pClientNode->m_pNextClient != NULL) {
//		if (!pClientNode->m_pNextClient->m_bIsMainSocketServer) {		// 子socket
//			DWORD bRet = wcscmp(pClient->m_lpszIpAddress, pClientNode->m_pNextClient->m_lpszIpAddress);
//			if (bRet == 0) {// IP相同
//				theApp.m_Server.m_pTcpPackServer->Disconnect(pClientNode->m_pNextClient->m_dwConnectId);
//			}
//		}
//		pClientNode = pClientNode->m_pNextClient;
//	}
//}




//
//
//// 以下函数为操作 存放CSocketClient的链表 的函数
//
//
//// 双向链表，新结点插在列表尾部
//VOID CClientManage::AddNewChildSocketClientToList(CSocketClient *pSocketClient) {
//	EnterCriticalSection(&m_ChildSocketClientLock);
//
//	m_pChildSocketClientListTail->m_pNextChildSocketClient = pSocketClient;		// 链表为空时m_pChildSocketClientListTail=m_pChildSocketClientListHead
//	pSocketClient->m_pLastChildSocketClient = m_pChildSocketClientListTail;
//	m_pChildSocketClientListTail = pSocketClient;
//
//	m_dwChildSocketClientNum++;
//
//#ifdef _DEBUG
//	USES_CONVERSION;
//	printf("[子socket上线 - %s:%d] 本IP共%d条子socket连接\n",
//		W2A(pSocketClient->m_lpszIpAddress), pSocketClient->m_wPort, m_dwChildSocketClientNum);
//#endif
//
//	LeaveCriticalSection(&m_ChildSocketClientLock);
//}
//
//
//// 删除SocketClient, 返回是否删除成功（删除失败主要原因可能是链表中并没有这个SocketClient）
//BOOL CClientManage::DeleteChildSocketClientFromList(CONNID dwConnectId) {
//
//	CSocketClient *pSocketClient = SearchSocketClient(dwConnectId);
//	if (pSocketClient == nullptr) {
//		return false;
//	}
//	else {
//		DeleteChildSocketClientFromList(pSocketClient);
//		return true;
//	}
//}
//
//
//// 内含CSocketClient的析构，所以有令子socket下线的作用
//VOID CClientManage::DeleteChildSocketClientFromList(CSocketClient *pSocketClient) {
//
//
//	EnterCriticalSection(&m_ChildSocketClientLock);
//
//	if (pSocketClient == m_pChildSocketClientListTail) {			// 要删除的结点是最后一个结点
//		pSocketClient->m_pLastChildSocketClient->m_pNextChildSocketClient = nullptr;
//		m_pChildSocketClientListTail = pSocketClient->m_pLastChildSocketClient;
//		pSocketClient->m_pLastChildSocketClient = nullptr;
//	}
//	else {
//		pSocketClient->m_pLastChildSocketClient->m_pNextChildSocketClient = pSocketClient->m_pNextChildSocketClient;
//		pSocketClient->m_pNextChildSocketClient->m_pLastChildSocketClient = pSocketClient->m_pLastChildSocketClient;
//		pSocketClient->m_pLastChildSocketClient = nullptr;
//		pSocketClient->m_pNextChildSocketClient = nullptr;
//	}
//
//	m_dwChildSocketClientNum--;
//
//#ifdef _DEBUG
//	USES_CONVERSION;			// 使用A2W之前先声明这个
//	printf("[子socket下线 - %s:%d] 本IP共%d条子socket连接\n",
//		W2A(pSocketClient->m_lpszIpAddress), pSocketClient->m_wPort, m_dwChildSocketClientNum);
//#endif
//
//	delete pSocketClient;				// SocketClient在这里释放内存
//
//	LeaveCriticalSection(&m_ChildSocketClientLock);
//}
//
//
//// 内含CSocketClient的析构，所以有令全部子socket下线的作用
//VOID CClientManage::DeleteAllChildSocketClientFromList() {
//	// 从链表尾部开始删
//	while (m_pChildSocketClientListTail != m_pChildSocketClientListHead) {
//		DeleteChildSocketClientFromList(m_pChildSocketClientListTail);
//	}
//}
//
//
////// SocketClient是否存在于链表中，存在则返回SocketClient地址，不存在返回nullptr
////CSocketClient* CClientManage::SearchSocketClient(CONNID dwConnectId) {
////
////
////	CSocketClient* Ret = nullptr;
////
////	__try {
////		EnterCriticalSection(&m_ChildSocketClientLock);
////
////		CSocketClient *pSocketClientNode = m_pChildSocketClientListHead;
////		while (pSocketClientNode->m_pNextChildSocketClient != nullptr) {
////			if (pSocketClientNode->m_pNextChildSocketClient->m_dwConnectId == dwConnectId) {
////				Ret = pSocketClientNode->m_pNextChildSocketClient;
////				__leave;			// 原本这里直接返回，没有释放锁。。。坑死我了
////			}
////			pSocketClientNode = pSocketClientNode->m_pNextChildSocketClient;
////		}
////
////	}
////	__finally {
////		LeaveCriticalSection(&m_ChildSocketClientLock);
////	}
////
////	return Ret;
////}
//
//
//
//
//
//// 下面的函数同时用到两个链表
//
//// 查找CSocketClient是否存在。分别遍历ClientList链表（找主socket）和ChildSocketClientList（找子socket）
//CSocketClient* CClientManage::SearchSocketClient(CONNID dwConnectId) {
//	CClient *pClientNode = m_pClientListHead;
//	while (pClientNode->m_pNextClient != nullptr) {
//		if (pClientNode->m_pNextClient->m_pMainSocketClient->m_dwConnectId == dwConnectId) {
//			return pClientNode->m_pNextClient->m_pMainSocketClient;
//		}
//		pClientNode = pClientNode->m_pNextClient;
//	}
//
//	CSocketClient* pChildSocketClientNode = m_pChildSocketClientListHead;
//	while (pChildSocketClientNode->m_pNextChildSocketClient != nullptr) {
//		if (pChildSocketClientNode->m_pNextChildSocketClient->m_dwConnectId == dwConnectId) {
//			return pChildSocketClientNode->m_pNextChildSocketClient;
//		}
//		pChildSocketClientNode = pChildSocketClientNode->m_pNextChildSocketClient;
//	}
//
//	return nullptr;
//}
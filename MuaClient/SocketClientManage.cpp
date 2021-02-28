#include "pch.h"
#include "SocketClientManage.h"


class CSocketClient;



CSocketClientManage::CSocketClientManage() {

	// 初始化链表，链表头结点是空结点。
	m_pSocketClientListHead = new CSocketClient();
	m_pSocketClientListTail = m_pSocketClientListHead;

	m_dwClientNum = 0;
	m_dwMainSocketClientNum = 0;
	m_dwChildSocketClientNum = 0;

	InitializeCriticalSection(&m_Lock);
}


CSocketClientManage::~CSocketClientManage() {
	DeleteCriticalSection(&m_Lock);
}


// 双向链表，新结点插在列表尾部
VOID CSocketClientManage::AddNewSocketClientToList(CSocketClient *pSocketClient) {
	EnterCriticalSection(&m_Lock);

	m_pSocketClientListTail->m_pNextSocketClient = pSocketClient;		// 链表为空时m_pClientListTail=m_pClientListHead
	pSocketClient->m_pLastSocketClient = m_pSocketClientListTail;
	m_pSocketClientListTail = pSocketClient;

	m_dwClientNum++;
	if (pSocketClient->m_bIsMainSocketClient) {
		m_dwMainSocketClientNum++;
	}
	else {
		m_dwChildSocketClientNum++;
	}

	LeaveCriticalSection(&m_Lock);
}


// 删除Client, 返回是否删除成功（删除失败主要原因可能是链表中并没有这个Client）
BOOL CSocketClientManage::DeleteSocketClientFromList(CONNID dwConnectId) {
	CSocketClient *pSocketClient = SearchSocketClient(dwConnectId);
	if (pSocketClient == NULL) {
		return false;
	}
	else {
		DeleteSocketClientFromList(pSocketClient);
		return true;
	}
}


VOID CSocketClientManage::DeleteSocketClientFromList(CSocketClient *pSocketClient) {
	EnterCriticalSection(&m_Lock);

	if (pSocketClient == m_pSocketClientListTail) {			// 要删除的结点是最后一个结点
		pSocketClient->m_pLastSocketClient->m_pNextSocketClient = NULL;
		m_pSocketClientListTail = pSocketClient->m_pLastSocketClient;
		pSocketClient->m_pLastSocketClient = NULL;
	}
	else {
		pSocketClient->m_pLastSocketClient->m_pNextSocketClient = pSocketClient->m_pNextSocketClient;
		pSocketClient->m_pNextSocketClient->m_pLastSocketClient = pSocketClient->m_pLastSocketClient;
		pSocketClient->m_pLastSocketClient = NULL;
		pSocketClient->m_pNextSocketClient = NULL;
	}

	m_dwClientNum--;
	if (pSocketClient->m_bIsMainSocketClient) {
		m_dwMainSocketClientNum--;
	}
	else {
		m_dwChildSocketClientNum--;
	}

	delete pSocketClient;				// Client在这里释放内存

	LeaveCriticalSection(&m_Lock);
}


VOID CSocketClientManage::DeleteAllSocketClientFromList() {
	// 从链表尾部开始删
	while (m_pSocketClientListTail != m_pSocketClientListHead) {
		DeleteSocketClientFromList(m_pSocketClientListTail);
	}
}


// Client是否存在于链表中，存在则返回Client地址，不存在返回NULL
CSocketClient* CSocketClientManage::SearchSocketClient(CONNID dwConnectId) {
	CSocketClient *pSocketClientNode = m_pSocketClientListHead;
	while (pSocketClientNode->m_pNextSocketClient != NULL) {
		if (pSocketClientNode->m_pNextSocketClient->m_dwConnectId == dwConnectId) {
			return pSocketClientNode->m_pNextSocketClient;
		}
		pSocketClientNode = pSocketClientNode->m_pNextSocketClient;
	}
	return NULL;
}
#include "pch.h"
#include "ClientManage.h"


// 以下是CClient的命名空间


CClient::CClient(CONNID dwConnectId, LPWSTR lpszAddress, WORD usPort) {
	m_dwConnectId			= dwConnectId;
	wcscpy_s(m_lpszIpAddress, lpszAddress);
	m_wPort					= usPort;

	// 目前先暂定被控端发来的第一个包是传递密钥的封包,接收密钥后，状态就是等待上线包
	m_dwClientStatus		= WAIT_FOR_LOGIN;

	m_pLastClient			= NULL;
	m_pNextClient			= NULL;
}


CClient::CClient() {
	m_dwConnectId			= 0;
	ZeroMemory(&m_lpszIpAddress, 20);
	m_wPort					= 0;
	m_dwClientStatus		= NOT_ONLINE;
	m_pLastClient			= NULL;
	m_pNextClient			= NULL;
}


CClient::~CClient() {
}


// 设置密钥（key和iv）
VOID CClient::SetCryptoKey(PBYTE pbCryptoKey, PBYTE pbCryptoIv) {
	m_Crypto = CCrypto(AES_128_CFB, pbCryptoKey, pbCryptoIv);
}


VOID CClient::Login() {

}






// 以下是CClientManage的命名空间
CClientManage::CClientManage() {

	// 初始化链表，链表头结点是空结点。

	//m_pClientListHead = &CClient();
	// 上面的写法申请的是局部变量，这个类对象在本函数结束时就被析构了，坑死了我。
	// 析构完了之后，m_pClientListHead执向的地址不再是类对象，后续的逻辑运行都是不可预料的了。
	m_pClientListHead = new CClient();
	m_pClientListTail = m_pClientListHead;

	m_dwClientNum = 0;

	InitializeCriticalSection(&m_Lock);
}


CClientManage::~CClientManage() {
	DeleteCriticalSection(&m_Lock);
}


// 双向链表，新结点插在列表尾部
VOID CClientManage::AddNewClientToList(CClient *pClient) {
	EnterCriticalSection(&m_Lock);

	m_pClientListTail->m_pNextClient = pClient;		// 链表为空时m_pClientListTail=m_pClientListHead
	pClient->m_pLastClient = m_pClientListTail;
	m_pClientListTail = pClient;
	m_dwClientNum++;
		
#ifdef DEBUG
	CHAR szIP[20];
	myW2A(pClient->m_lpszIpAddress, szIP, 20);
	printf("[上线] IP=%s, PORT=%d, 当前共%d个客户端在线\n", szIP, pClient->m_wPort, m_dwClientNum);
#endif

	LeaveCriticalSection(&m_Lock);
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
	EnterCriticalSection(&m_Lock);

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

#ifdef DEBUG
	CHAR szIP[20];
	myW2A(pClient->m_lpszIpAddress, szIP, 20);
	printf("[下线] IP=%s, PORT=%d, 当前共%d个客户端在线\n", szIP, pClient->m_wPort, m_dwClientNum);
#endif

	delete pClient;				// Client在这里释放内存

	LeaveCriticalSection(&m_Lock);
}


VOID CClientManage::DeleteAllClientFromList() {
	// 从链表尾部开始删
	while (m_pClientListTail != m_pClientListHead) {
		DeleteClientFromList(m_pClientListTail);
	}
}


// Client是否存在于链表中，存在则返回Client地址，不存在返回NULL
CClient* CClientManage::SearchClient(CONNID dwConnectId) {
	CClient *pClientNode = m_pClientListHead;
	while (pClientNode->m_pNextClient != NULL) {
		if (pClientNode->m_pNextClient->m_dwConnectId == dwConnectId) {
			return pClientNode->m_pNextClient;
		}
		pClientNode = pClientNode->m_pNextClient;
	}
	return NULL;
}
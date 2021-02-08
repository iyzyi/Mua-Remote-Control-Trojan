#include "pch.h"
#include "ClientManage.h"


// 以下是CClient的命名空间


CClient::CClient(CONNID dwConnectId, LPWSTR lpszAddress, WORD usPort) {
	m_dwConnectId			= dwConnectId;
	wcscpy_s(m_lpszIpAddress, lpszAddress);
	m_wPort					= usPort;

	m_dwClientStatus		= HAVE_CRYPTO_KEY;

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
}


CClientManage::~CClientManage() {
}


// 双向链表，新结点插在列表尾部
VOID CClientManage::AddNewClientToList(CClient *pClient) {
		m_pClientListTail->m_pNextClient = pClient;		// 链表为空时m_pClientListTail=m_pClientListHead
		pClient->m_pLastClient = m_pClientListTail;
		m_pClientListTail = pClient;
}


// 删除Client, 返回是否删除成功（删除失败主要原因可能是链表中并没有这个Client）
BOOL CClientManage::DeleteClientFromList(CONNID dwConnectId) {
	CClient *pClient = SearchClient(dwConnectId);
	if (pClient == NULL) {
		return false;
	}
	else {
		pClient->m_pLastClient->m_pNextClient = pClient->m_pNextClient;
		pClient->m_pNextClient->m_pLastClient = pClient->m_pLastClient;
		pClient->m_pLastClient = NULL;
		pClient->m_pNextClient = NULL;
		return true;
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
#include "pch.h"
#include "ClientManage.h"


// 以下是CClient的命名空间


// 只能在接收到密钥和IV时初始化，并将新增的CClient对象加入CClientManage对象
CClient::CClient(CONNID dwConnectId) {
	m_dwConnectId			= dwConnectId;
	m_dwClientStatus		= HAVE_CRYPTO_KEY;
	m_pNextClient			= NULL;
}


CClient::~CClient() {

}


VOID CClient::SetCryptoKey(PBYTE pbCryptoKey = NULL, PBYTE pbCryptoIv = NULL) {
	m_Crypto = CCrypto(AES_128_CFB, pbCryptoKey, pbCryptoIv);
}


VOID CClient::Login() {

}






// 以下是CClientManage的命名空间
CClientManage::CClientManage() {
	m_pClientListHead = NULL;
}


CClientManage::~CClientManage() {

}

// 单向链表，所以为了性能自然是默认将新的结点添加到链表头部了
VOID CClientManage::AddNewClientToList(CClient *pClient) {
	if (m_pClientListHead->m_pNextClient) {		// 链表目前为空
		m_pClientListHead->m_pNextClient = pClient;
	}
	else {										// 链表不为空
		pClient->m_pNextClient = m_pClientListHead->m_pNextClient;
		m_pClientListHead->m_pNextClient = pClient;
	}
}


// 删除Client, 返回是否删除成功（删除失败主要原因可能是链表中并没有这个Client）
BOOL CClientManage::DeleteClientFromList(CClient *pClient) {
	DWORD dwConnectId = pClient->m_dwConnectId;
	CClient *pClient
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
#include "pch.h"
#include "ClientManage.h"


// 以下是CClient的命名空间


// 只能在接收到密钥和IV时初始化，并将新增的CClient对象加入CClientManage对象
CClient::CClient(CONNID dwConnectId) {
	m_dwConnectId			= dwConnectId;
	m_dwClientStatus		= HAVE_CRYPTO_KEY;
	
	m_pLastClient			= NULL;
	m_pNextClient			= NULL;
}


CClient::~CClient() {

}


VOID CClient::SetCryptoKey(PBYTE pbCryptoKey, PBYTE pbCryptoIv) {
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

// 双向循环链表，新结点插在列表尾部
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
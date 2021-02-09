#include "pch.h"
#include "SocketServer.h"


// 16字节Key+16字节的IV
#define FIRST_PACKET_LENGTH 32


CSocketServer::CSocketServer() : m_pServer(this) {
	m_bIsRunning = false;
	m_pfnManageRecvPacket = NULL;

	// 设置数据包最大长度（有效数据包最大长度不能超过0x3FFFFF字节(4MB-1B)，默认：262144/0x40000 (256KB)
	m_pServer->SetMaxPackSize(PACKET_MAX_LENGTH);
	// 设置心跳检测包发送间隔
	m_pServer->SetKeepAliveTime(60 * 1000);
	// 设置心跳检测重试包发送间隔
	m_pServer->SetKeepAliveInterval(20 * 1000);
}


CSocketServer::~CSocketServer() {

}


// 启动socket服务端
BOOL CSocketServer::StartSocketServer(NOTIFYPROC pfnNotifyProc, LPCTSTR lpszIpAddress, USHORT wPort) {

	BOOL bRet = m_pServer->Start(lpszIpAddress, wPort);
	if (!bRet) {
		return false;
	} else {
#ifdef _DEBUG
		char szIP[50];
		//WideCharToMultiByte(CP_ACP, 0, lpszIpAddress, -1, szIP, 50, NULL, NULL);
		myW2A(lpszIpAddress, szIP, 50);
		printf("Socket服务端启动成功，IP=%s, PORT=%d\n", szIP, wPort);
#endif

		// 设置回调函数
		m_pfnManageRecvPacket = pfnNotifyProc;

		// 初始化ClientManage的Client链表
		m_ClientManage = CClientManage();

		m_bIsRunning = true;
		return true;
	}
}


BOOL CSocketServer::StopSocketServer() {
	BOOL bRet = m_pServer->Stop();
	if (bRet) {
		m_bIsRunning = false;
		return true;
	}
	else {
		return false;
	}
}




BOOL CSocketServer::SendPacket(CONNID dwConnectId, COMMAND_ID dwCommandId, PBYTE pbPacketBody, DWORD dwPacketBodyLength) {
	BOOL bRet;
	CClient *pClient = m_ClientManage.SearchClient(dwConnectId);
	if (pClient != NULL) {
		bRet = SendPacket(pClient, dwCommandId, pbPacketBody, dwPacketBodyLength);
	} else {
		bRet = false;
	}
	return bRet;
}


BOOL CSocketServer::SendPacket(CClient* pClient, COMMAND_ID dwCommandId, PBYTE pbPacketBody, DWORD dwPacketBodyLength) {
	// 发包只需要ConnectId就能发，但是通信的密钥在CClient类对象里面，
	// CPacket的封包加密需要CClient里面的密钥，所以必须传入CClient参数。

	CPacket Packet = CPacket(pClient);
	Packet.PacketCombine(dwCommandId, pbPacketBody, dwPacketBodyLength);
	BOOL bRet = m_pServer->Send(pClient->m_dwConnectId, Packet.m_pbPacketCiphertext, Packet.m_dwPacketLength);
	return bRet;
}


VOID CSocketServer::SendPacketToAllClient(COMMAND_ID dwCommandId, PBYTE pbPacketBody, DWORD dwPacketBodyLength) {
	CClient *pClientNode = m_ClientManage.m_pClientListHead;
	while (pClientNode->m_pNextClient != NULL) {
		SendPacket(pClientNode->m_pNextClient, dwCommandId, pbPacketBody, dwPacketBodyLength);
		pClientNode = pClientNode->m_pNextClient;
	}
}




BOOL CSocketServer::IsRunning() {
	return m_bIsRunning;
}




// 回调函数的实现

EnHandleResult CSocketServer::OnPrepareListen(ITcpServer* pSender, SOCKET soListen) {
	printf("OnPrepareListen: \n");
	return HR_OK;
}


EnHandleResult CSocketServer::OnAccept(ITcpServer* pSender, CONNID dwConnID, SOCKET soClient) {
	printf("[Client %d] OnAccept: \n", dwConnID);
	return HR_OK;
}


EnHandleResult CSocketServer::OnHandShake(ITcpServer* pSender, CONNID dwConnID) {
	printf("[Client %d] OnHandShake: \n", dwConnID);
	return HR_OK;
}


EnHandleResult CSocketServer::OnSend(ITcpServer* pSender, CONNID dwConnID, const BYTE* pData, int iLength) {
	printf("[Client %d] OnSend: \n", dwConnID);
	//PrintBytes((LPBYTE)pData, iLength);
	return HR_OK;
}


EnHandleResult CSocketServer::OnReceive(ITcpServer* pSender, CONNID dwConnID, const BYTE* pData, int iLength) {
	printf("[Client %d] OnReceive: \n", dwConnID);
	PrintBytes((PBYTE)pData, iLength);
	
	CClient* pClient = m_ClientManage.SearchClient(dwConnID);
	if (pClient == NULL) {						// 新客户端来啦

		if (iLength == FIRST_PACKET_LENGTH) {	// 第一个封包是AES的key和iv，所以长度必须满足这个条件。否则丢弃该包，以免拒绝服务。

			TCHAR lpszIpAddress[20];
			int iIpAddressLen = 20;
			WORD wPort = 0;
			// 通过ConnectId获取IP地址和端口
			m_pServer->GetRemoteAddress(dwConnID, lpszIpAddress, iIpAddressLen, wPort);

			CClient* pClientNew = new CClient(dwConnID, (LPWSTR)lpszIpAddress, wPort);
			m_ClientManage.AddNewClientToList(pClientNew);

			// 设置该Client的密钥
			BYTE pbKey[16];
			BYTE pbIv[16];
			memcpy(pbKey, pData, 16);
			memcpy(pbIv, pData + 16, 16);
			pClientNew->SetCryptoKey(pbKey, pbIv);

			SendPacket(pClientNew, CRYPTO_KEY, NULL, 0);

		} // if (iLength == FIRST_PACKET_LENGTH)

	} // if (pClient == NULL)
	else {

		CPacket Packet = CPacket(pClient);
		BOOL isValidPacket = Packet.PacketParse((PBYTE)pData, iLength);

		if (isValidPacket) {								// 有效封包

			switch (pClient->m_dwClientStatus) {			// 客户端的不同状态

			case WAIT_FOR_LOGIN:							// 服务端已经接收了客户端发来的密钥了，等待上线包

				/*if (Packet.m_PacketHead.wCommandId == LOGIN) {
					CHAR szMsg[] = "Hello Everyone, I am the Mua Server!";
					SendPacket(pClient, ECHO, (PBYTE)szMsg, strlen(szMsg));
				}*/
				// 这个阶段，只要不是上线包，通通丢弃。

			case LOGINED:									// 接收上线包后，状态变为已登录
				;
			}
		}
	}

	return HR_OK;

	/*CPacket Packet = CPacket(dwConnID);
	Packet.PacketParse((PBYTE)pData, (DWORD)iLength);

	m_pfnManageRecvPacket(Packet);
*/


	//PacketParse((PBYTE)pData, iLength);
	//BOOL bRet = m_pServer->Send(dwConnID, pData, iLength);
	//return bRet ? HR_OK : HR_ERROR;
	
	//if (pData[0] == 'A') {
	//	BYTE Buffer[] = "I am iyzyi!";
	//	PBYTE pbData = CopyBuffer(Buffer, 11);		// 不这样多加一层，xfree(m_pPacketBody)直接崩。
	//	SendPacket(dwConnID, FILE_TRANSFOR, pbData, 11);
	//}

}


EnHandleResult CSocketServer::OnClose(ITcpServer* pSender, CONNID dwConnID, EnSocketOperation enOperation, int iErrorCode) {
	printf("[Client %d] OnClose: \n", dwConnID);

	m_ClientManage.DeleteClientFromList(dwConnID);

	return HR_OK;
}


EnHandleResult CSocketServer::OnShutdown(ITcpServer* pSender) {
	printf("OnShutdown: \n");
	return HR_OK;
}
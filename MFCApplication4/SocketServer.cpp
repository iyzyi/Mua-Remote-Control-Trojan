#include "pch.h"
#include "SocketServer.h"




CSocketServer::CSocketServer() : m_pServer(this) {
	m_bIsRunning = false;
	m_pfnManageRecvPacket = NULL;
}


CSocketServer::~CSocketServer() {

}


// 初始化socket服务端
BOOL CSocketServer::StartSocketServer(NOTIFYPROC pfnNotifyProc, LPCTSTR lpszIpAddress, USHORT wPort) {
	// 设置数据包最大长度（有效数据包最大长度不能超过0x3FFFFF字节(4MB-1B)，默认：262144/0x40000 (256KB)
	m_pServer->SetMaxPackSize(PACKET_MAX_LENGTH);
	// 设置心跳检测包发送间隔
	m_pServer->SetKeepAliveTime(60 * 1000);				
	// 设置心跳检测重试包发送间隔
	m_pServer->SetKeepAliveInterval(20 * 1000);			

	BOOL bRet = m_pServer->Start(lpszIpAddress, wPort);
	if (!bRet) {
		return false;
	} else {
#ifdef DEBUG
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
	CPacket Packet = CPacket(dwConnectId);
	Packet.PacketCombine(dwCommandId, pbPacketBody, dwPacketBodyLength);
	BOOL bRet = m_pServer->Send(dwConnectId, Packet.m_pbPacketCipherData, Packet.m_dwPacketLength);
	return bRet;
}


VOID CSocketServer::SendPacketToAllClient(COMMAND_ID dwCommandId, PBYTE pbPacketBody, DWORD dwLength) {
	
}






// 与Client连接的链表

VOID CSocketServer::ListAddClient(CONNID ConnectId) {
	//m_ClientList.insertNodeByhead(ConnectId);
}


VOID CSocketServer::ListDeleteClient(CONNID ConnectId) {
	;
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

		TCHAR lpszIpAddress[20];
		int iIpAddressLen = 20;
		WORD wPort = 0;
		// 通过ConnectId获取IP地址和端口
		m_pServer->GetRemoteAddress(dwConnID, lpszIpAddress, iIpAddressLen, wPort);

		CClient* pClientNew = new CClient(dwConnID, (LPWSTR)lpszIpAddress, wPort);
		m_ClientManage.AddNewClientToList(pClientNew);
	} 
	else {
		switch (pClient->m_dwClientStatus) {			// 客户端的不同状态

		case WAIT_FOR_LOGIN:							// 服务端已经接收了客户端发来的密钥了，等待上线包
			;

		case LOGINED:									// 接收上线包后，状态变为已登录
			;
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
#include "pch.h"
#include "SocketServer.h"
#include "Crypto.h"


//#define ADDRESS (L"0.0.0.0")
//#define PORT ((USHORT)(5555))



CSocketServer::CSocketServer() : m_Server(this) {
	m_bIsRunning = false;

	m_pfnManageRecvPacket = NULL;
}


CSocketServer::~CSocketServer() {

}


// 初始化socket服务端
BOOL CSocketServer::StartSocketServer(NOTIFYPROC pfnNotifyProc, LPCTSTR lpszIpAddress, USHORT wPort) {
	// 设置数据包最大长度（有效数据包最大长度不能超过0x3FFFFF字节(4MB-1B)，默认：262144/0x40000 (256KB)
	m_Server->SetMaxPackSize(PACKET_MAX_LENGTH);

	//m_Server->SetKeepAliveTime();				// 设置心跳检测包发送间隔
	//m_Server->SetKeepAliveInterval();			// 设置心跳检测重试包发送间隔

	BOOL bRet = m_Server->Start(lpszIpAddress, wPort);
	if (bRet) {

#ifdef DEBUG
		char szIP[50];
		WideCharToMultiByte(CP_ACP, 0, lpszIpAddress, -1, szIP, 50, NULL, NULL);
		printf("Socket服务端启动成功，IP=%s, PORT=%d\n", szIP, wPort);
#endif

		m_pfnManageRecvPacket = pfnNotifyProc;

		m_bIsRunning = true;
		return true;
	}
	else {
		return false;
	}
}


BOOL CSocketServer::StopSocketServer() {
	BOOL bRet = m_Server->Stop();
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
	BOOL bRet = m_Server->Send(dwConnectId, Packet.m_pbPacketCipherData, Packet.m_dwPacketLength);
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


EnHandleResult CSocketServer::OnAccept(ITcpServer* pSender, CONNID dwConnID, SOCKET soClient)
{
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
	
	//PacketParse((PBYTE)pData, iLength);
	//BOOL bRet = m_Server->Send(dwConnID, pData, iLength);
	//return bRet ? HR_OK : HR_ERROR;
	
	//if (pData[0] == 'A') {
	//	BYTE Buffer[] = "I am iyzyi!";
	//	PBYTE pbData = CopyBuffer(Buffer, 11);		// 不这样多加一层，xfree(m_pPacketBody)直接崩。
	//	SendPacket(dwConnID, FILE_TRANSFOR, pbData, 11);
	//}
	
	CPacket Packet = CPacket(dwConnID);
	Packet.PacketParse((PBYTE)pData, (DWORD)iLength);

	m_pfnManageRecvPacket(Packet);
	return HR_OK;
}


EnHandleResult CSocketServer::OnClose(ITcpServer* pSender, CONNID dwConnID, EnSocketOperation enOperation, int iErrorCode) {
	printf("[Client %d] OnClose: \n", dwConnID);
	return HR_OK;
}


EnHandleResult CSocketServer::OnShutdown(ITcpServer* pSender) {
	printf("OnShutdown: \n");
	return HR_OK;
}
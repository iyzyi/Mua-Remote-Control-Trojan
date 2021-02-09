#pragma once
#include "pch.h"

// 这个应该在Packet.h中宏定义，暂时先放在这
// 封包长度受HP-Socket的限制，最大为0x3FFFFF
#define PACKET_MAX_LENGTH 0x3FFFFF


class CSocketClient : public CTcpClientListener {

public:
	CTcpPackClientPtr		m_pClient;

public:

	CSocketClient();
	~CSocketClient();

	BOOL StartSocketClient();



	// 重写回调函数
	virtual EnHandleResult OnSend(ITcpClient* pSender, CONNID dwConnID, const BYTE* pData, int iLength);
	virtual EnHandleResult OnReceive(ITcpClient* pSender, CONNID dwConnID, const BYTE* pData, int iLength);
	virtual EnHandleResult OnClose(ITcpClient* pSender, CONNID dwConnID, EnSocketOperation enOperation, int iErrorCode);
	virtual EnHandleResult OnConnect(ITcpClient* pSender, CONNID dwConnID);
};
#pragma once
#include "pch.h"
#include "Crypto.h"


class CSocketClient : public CTcpClientListener {

public:
	CTcpPackClientPtr		m_pClient;

	CCrypto					m_Crypto;

	CLIENT_STATUS			m_dwClientStatus;

public:

	CSocketClient();
	~CSocketClient();

	BOOL StartSocketClient();

	BOOL SendPacket(COMMAND_ID dwCommandId, PBYTE pbPacketBody, DWORD dwPacketBodyLength);



	// 重写回调函数
	virtual EnHandleResult OnPrepareConnect(ITcpClient* pSender, CONNID dwConnID, SOCKET socket);
	virtual EnHandleResult OnConnect(ITcpClient* pSender, CONNID dwConnID);
	virtual EnHandleResult OnHandShake(ITcpClient* pSender, CONNID dwConnID);
	virtual EnHandleResult OnSend(ITcpClient* pSender, CONNID dwConnID, const BYTE* pData, int iLength);
	virtual EnHandleResult OnReceive(ITcpClient* pSender, CONNID dwConnID, const BYTE* pData, int iLength);
	virtual EnHandleResult OnClose(ITcpClient* pSender, CONNID dwConnID, EnSocketOperation enOperation, int iErrorCode);

};
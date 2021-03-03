#pragma once

#include "Packet.h"
#include "ClientManage.h"

class CModule {
public:
	CSocketClient*		m_pSocketClient;
	CClient*			m_pClient;

public:
	CModule(CSocketClient* pSocketClient);
	~CModule();
	VOID InitModule(CSocketClient* pSocketClient);

	virtual void OnRecvChildSocketClientPacket(CPacket* pPacket);
};



void RunShellRemote(CSocketClient* pClient);
void RunFileUpload(CSocketClient* pClient);
#pragma once

//VOID OnRecvShellRemotePacket(CPacket* pPacket);

#include "Packet.h"
#include "ClientManage.h"

class CModule {
public:
	CClient*		m_pClient;

public:
	CModule(CClient* pClient);
	~CModule();

	virtual void OnRecvChildSocketClientPacket(CPacket* pPacket);
};



void RunShellRemote(CClient* pClient);
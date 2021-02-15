#pragma once

#include "ModuleManage.h"


class CFileUpload : public  CModule {
public:
	CFileUpload(CSocketClient* pClient);
	~CFileUpload();

	// ÖØÐ´Ðéº¯Êý
	void OnRecvChildSocketClientPacket(CPacket* pPacket);
};

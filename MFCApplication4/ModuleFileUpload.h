#pragma once

#include "ModuleManage.h"


class CFileUpload : public  CModule {
public:
	CFileUpload(CClient* pClient);
	~CFileUpload();

	// ÖØÐ´Ðéº¯Êý
	void OnRecvChildSocketClientPacket(CPacket* pPacket);
};

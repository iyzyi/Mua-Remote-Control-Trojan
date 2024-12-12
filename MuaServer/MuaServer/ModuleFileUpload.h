#pragma once

#include "ModuleManage.h"



#define FILE_UPLOAD_INFO_PACKET_BODY_LENGTH (16 + MAX_PATH * 2)



class CFileUpload : public  CModule {
public:
	WCHAR						m_pszFilePath[MAX_PATH];			// 要上传的文件的本地路径

	WCHAR						m_pszUploadPath[MAX_PATH];			// 要上传到被控端的什么目录下

	HANDLE						m_hRecvPacketFileUploadInfoEvent;
	HANDLE						m_hRecvPacketFileUploadCloseEvent;

public:
	CFileUpload(CSocketClient* pSocketClient);
	~CFileUpload();

	// 重写虚函数
	void OnRecvChildSocketClientPacket(CPacket* pPacket);
};


BOOL UploadFile(CClient* pClient, LPWSTR pszFilePath, LPWSTR pszUploadPath);
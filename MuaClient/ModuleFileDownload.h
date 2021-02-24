#pragma once

#include "SocketClient.h"



class CModuleFileDownload : public  CModule {
public:
	WCHAR						m_pszLocalPath[MAX_PATH];			// 主控端要下载的被控端文件路径

	//WCHAR						m_psRemotePath[MAX_PATH];			// 要下载到主控端的哪个路径下

	HANDLE						m_hRecvPacketFileDownloadInfoEvent;
	HANDLE						m_hRecvPacketFileDownloadCloseEvent;

	HANDLE						m_hFile;

public:
	CModuleFileDownload(CSocketClient* pSocketClient);
	~CModuleFileDownload();

	// 重写虚函数
	void OnRecvivePacket(CPacket* pPacket);
};


BOOL WINAPI OnRecvPacketFileDownloadInfo(LPVOID lParam);
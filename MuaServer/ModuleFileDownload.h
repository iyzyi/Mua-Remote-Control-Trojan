#pragma once

#include "ModuleManage.h"



#define FILE_DOWNLOAD_INFO_PACKET_BODY_LENGTH (MAX_PATH * 2)



class CModuleFileDownload : public  CModule {
public:
	CRITICAL_SECTION			m_WriteLock;

	QWORD						m_qwFileSize;
	DWORD						m_dwPacketSplitNum;

	WCHAR						m_pszRemotePath[MAX_PATH];			// 要下载的文件的路径（被控端文件的路径）
	WCHAR						m_pszLocalPath[MAX_PATH];			// 要下载到本地的什么目录下

	HANDLE						m_hRecvPacketFileDownloadInfoEvent;
	HANDLE						m_hRecvPacketFileDownloadCloseEvent;

	HANDLE						m_hFile;
	
	BYTE						m_byStatus;							// 被控端文件状态，如文件不存在之类的

public:
	CModuleFileDownload(CSocketClient* pSocketClient, PBYTE pszRemotePath, PBYTE pszLocalPath);
	~CModuleFileDownload();

	// 重写虚函数
	void OnRecvChildSocketClientPacket(CPacket* pPacket);
};


BOOL DownloadFile(CClient* pClient, LPWSTR pszRemotePath, LPWSTR pszLocalPath);


VOID WINAPI OnRecvPacketFileDownloadInfo(LPVOID lParam);
VOID WINAPI OnRecvPacketFileDownloadData(LPVOID lParam);
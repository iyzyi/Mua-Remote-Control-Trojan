#include "pch.h"
#include "ModuleFileDownload.h"
#include "MuaServer.h"


CModuleFileDownload::CModuleFileDownload(CSocketClient* pSocketClient, PBYTE pszRemotePath, PBYTE pszLocalPath) : CModule(pSocketClient) {
	// 第二个参数为true时表示手动重置事件
	m_hRecvPacketFileDownloadInfoEvent = CreateEvent(NULL, true, false, NULL);
	m_hRecvPacketFileDownloadCloseEvent = CreateEvent(NULL, true, false, NULL);

	memcpy(m_pszRemotePath, pszRemotePath, (wcslen((LPWSTR)pszRemotePath) + 1) * 2);
	memcpy(m_pszLocalPath, pszLocalPath, (wcslen((LPWSTR)pszLocalPath) + 1) * 2);

	InitializeCriticalSection(&m_WriteLock);
}


CModuleFileDownload::~CModuleFileDownload() {
	DeleteCriticalSection(&m_WriteLock);

	if (m_hRecvPacketFileDownloadInfoEvent) {
		CloseHandle(m_hRecvPacketFileDownloadInfoEvent);
		m_hRecvPacketFileDownloadInfoEvent = nullptr;
	}
	if (m_hRecvPacketFileDownloadCloseEvent) {
		CloseHandle(m_hRecvPacketFileDownloadCloseEvent);
		m_hRecvPacketFileDownloadCloseEvent = nullptr;
	}
	if (m_hFile) {
		CloseHandle(m_hFile);
		m_hFile = nullptr;
	}
}




BOOL WINAPI DownloadFileThreadFunc(LPVOID lParam);


typedef struct _DOWNLOAD_FILE_THREAD_PARAM {
	CClient*			pClient;
	WCHAR				m_pszRemotePath[MAX_PATH];		// 要下载的文件的路径（被控端文件的路径）
	WCHAR				m_pszLocalPath[MAX_PATH];		// 要下载到本地的什么目录下

	_DOWNLOAD_FILE_THREAD_PARAM(CClient* pClient, LPWSTR pszRemotePath, LPWSTR pszLocalPath) {
		this->pClient = pClient;
		memcpy(this->m_pszRemotePath, pszRemotePath, (wcslen(pszRemotePath) + 1) * 2);
		memcpy(this->m_pszLocalPath, pszLocalPath, (wcslen(pszLocalPath) + 1) * 2);
	}
}DOWNLOAD_FILE_THREAD_PARAM;


// TODO 开了线程怎么知道是否成功上传(即异步怎么知道处理结果，目前我想到的是处理结束后来个回调)
BOOL DownloadFile(CClient* pClient, LPWSTR pszRemotePath, LPWSTR pszLocalPath) {
	DOWNLOAD_FILE_THREAD_PARAM* lParam = new DOWNLOAD_FILE_THREAD_PARAM(pClient, pszRemotePath, pszLocalPath);
	CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)DownloadFileThreadFunc, (LPVOID)lParam, 0, NULL);
	return 0;
}


BOOL WINAPI DownloadFileThreadFunc(LPVOID lParam) {
	DOWNLOAD_FILE_THREAD_PARAM* pThreadParam = ((DOWNLOAD_FILE_THREAD_PARAM*)lParam);
	WCHAR pszRemotePath[MAX_PATH];
	WCHAR pszLocalPath[MAX_PATH];

	CClient* pClient = pThreadParam->pClient;
	memcpy(pszRemotePath, pThreadParam->m_pszRemotePath, (wcslen(pThreadParam->m_pszRemotePath) + 1) * 2);
	memcpy(pszLocalPath, pThreadParam->m_pszLocalPath, (wcslen(pThreadParam->m_pszLocalPath) + 1) * 2);

	if (lParam != nullptr) {
		delete lParam;
		lParam = nullptr;
	}

	// 选中客户端的主socket发送CONNECT包
	theApp.m_Server.SendPacket(pClient->m_pMainSocketClient, FILE_DOWNLOAD_CONNECT, NULL, 0);

	// 等待被控端发回CONNECT包
	WaitForSingleObject(pClient->m_FileDownloadConnectSuccessEvent, INFINITE);		// TODO 后续要把这里设置成等待有限时间

	ASSERT(pClient->m_pFileDownloadConnectSocketClientTemp != nullptr);
	CModuleFileDownload* pFileDownload = new CModuleFileDownload(pClient->m_pFileDownloadConnectSocketClientTemp, (PBYTE)pszRemotePath, (PBYTE)pszLocalPath);
	CSocketClient* pSocketClient = pFileDownload->m_pSocketClient;

	
	// 发往被控端的FILE_DOWNLOAD_INFO包体：要下载的被控端文件的路径（MAX_PATH*2 字节）
	BYTE pbPacketBody[FILE_DOWNLOAD_INFO_PACKET_BODY_LENGTH];
	ZeroMemory(pbPacketBody, sizeof(pbPacketBody));
	memcpy(pbPacketBody, pszRemotePath, MAX_PATH * 2);

	// 子socket发送FILE_DOWNLOAD_INFO包
	theApp.m_Server.SendPacket(pSocketClient, FILE_DOWNLOAD_INFO, (PBYTE)pbPacketBody, FILE_DOWNLOAD_INFO_PACKET_BODY_LENGTH);

	// 等待被控端发回FILE_DOWNLOAD_INFO包
	WaitForSingleObject(pFileDownload->m_hRecvPacketFileDownloadInfoEvent, INFINITE);


	// 传输文件数据是在收到FILE_DOWNLOAD_DATA包时，由回调函数另起线程来处理
	

	// 等待被控端发来FILE_DOWNLOAD_CLOSE包
	WaitForSingleObject(pFileDownload->m_hRecvPacketFileDownloadCloseEvent, INFINITE);

	theApp.m_Server.m_pTcpPackServer->Disconnect(pSocketClient->m_dwConnectId);

	if (pFileDownload != nullptr) {
		delete pFileDownload;
		pFileDownload = nullptr;
	}

	return true;
}



typedef struct _THREAD_PARAM {
	CPacket*				m_pPacket;
	CModuleFileDownload*			m_pModuleFileDownload;
	_THREAD_PARAM(CPacket* pPacket, CModuleFileDownload* pModuleFileDownload) {
		m_pPacket = pPacket;
		m_pModuleFileDownload = pModuleFileDownload;
	}
}THREAD_PARAM;


// 重写虚函数
void CModuleFileDownload::OnRecvChildSocketClientPacket(CPacket* pPacket) {
	CPacket* pPacketCopy = new CPacket(*pPacket);					// 记得在线程函数里面delete这个包
	THREAD_PARAM* lParam = new THREAD_PARAM(pPacketCopy, this);

	switch (pPacketCopy->m_PacketHead.wCommandId) {

	case FILE_DOWNLOAD_INFO:
		CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)OnRecvPacketFileDownloadInfo, (LPVOID)lParam, 0, NULL);
		break;

	case FILE_DOWNLOAD_DATA:
		CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)OnRecvPacketFileDownloadData, (LPVOID)lParam, 0, NULL);
		break;

	case FILE_DOWNLOAD_DATA_TAIL:
		CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)OnRecvPacketFileDownloadData, (LPVOID)lParam, 0, NULL);
		break;

	case FILE_DOWNLOAD_CLOSE:
		SetEvent(m_hRecvPacketFileDownloadCloseEvent);
		delete pPacketCopy;		// pPacket被外面的函数的__finally delete
		break;
	}
}


VOID WINAPI OnRecvPacketFileDownloadInfo(LPVOID lParam) {
	THREAD_PARAM* pThreadParam = (THREAD_PARAM*)lParam;
	CPacket* pPacket = pThreadParam->m_pPacket;
	CSocketClient* pSocketClient = pPacket->m_pSocketClient;
	CModuleFileDownload* pModuleFileDownload = pThreadParam->m_pModuleFileDownload;
	delete pThreadParam;
	pThreadParam = nullptr;

	// 接收到被控端发来的INFO包体：被控端文件状态（1字节）+ 文件大小（8字节）
	pModuleFileDownload->m_byStatus = GetByteFromBuffer(pPacket->m_pbPacketBody, 0);
	if (pModuleFileDownload->m_byStatus == 0xff) {		// 被控端文件打开失败
		SetEvent(pModuleFileDownload->m_hRecvPacketFileDownloadInfoEvent);
		MessageBox(0, L"被控端文件打开失败", L"被控端文件打开失败", 0);
		SetEvent(pModuleFileDownload->m_hRecvPacketFileDownloadCloseEvent);
	}
	else if (pModuleFileDownload->m_byStatus == 0) {	// 被控端文件打开成功
		pModuleFileDownload->m_qwFileSize = GetQwordFromBuffer(pPacket->m_pbPacketBody, 1);
		pModuleFileDownload->m_dwPacketSplitNum = (pModuleFileDownload->m_qwFileSize % PACKET_BODY_MAX_LENGTH) ? pModuleFileDownload->m_qwFileSize / PACKET_BODY_MAX_LENGTH + 1 : pModuleFileDownload->m_qwFileSize / PACKET_BODY_MAX_LENGTH;

		// 总是创建新文件
		pModuleFileDownload->m_hFile = CreateFile(pModuleFileDownload->m_pszLocalPath, GENERIC_WRITE, FILE_SHARE_READ, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

		if (pModuleFileDownload->m_hFile == INVALID_HANDLE_VALUE) {
			MessageBox(0, L"文件创建失败", L"文件创建失败", 0);
			// TODO 发送断开连接的封包
			theApp.m_Server.SendPacket(pSocketClient, FILE_DOWNLOAD_CLOSE, NULL, 0);
			return;
		}

		delete pPacket;

		SetEvent(pModuleFileDownload->m_hRecvPacketFileDownloadInfoEvent);
	}
}


VOID WINAPI OnRecvPacketFileDownloadData(LPVOID lParam) {
	THREAD_PARAM* pThreadParam = (THREAD_PARAM*)lParam;
	CPacket* pPacket = pThreadParam->m_pPacket;
	CSocketClient* pSocketClient = pPacket->m_pSocketClient;
	CModuleFileDownload* pModuleFileDownload = pThreadParam->m_pModuleFileDownload;
	delete pThreadParam;
	pThreadParam = nullptr;


	// 等待信号。
	HANDLE ahEvents[2] = { pModuleFileDownload->m_hRecvPacketFileDownloadInfoEvent, pModuleFileDownload->m_hRecvPacketFileDownloadCloseEvent };
	DWORD dwEvent = WaitForMultipleObjects(2, ahEvents, false, INFINITE);

	switch (dwEvent) {

	// 本Client收到过FILE_DOWNLOAD_INFO封包(这样才能继续处理FILE_DOWNLOAD_DATA和FILE_DOWNLOAD_DATA_TAIL)
	case WAIT_OBJECT_0 + 0: {

		EnterCriticalSection(&pModuleFileDownload->m_WriteLock);

		DWORD dwBytesWritten = 0;
		BOOL bRet = WriteFile(
			pModuleFileDownload->m_hFile,				// open file handle
			pPacket->m_pbPacketBody,				// start of data to write
			pPacket->m_dwPacketBodyLength,			// number of bytes to write
			&dwBytesWritten,						// number of bytes that were written
			NULL);									// no overlapped structure

		if (!bRet) {
			MessageBox(0, L"写入失败", L"写入失败", 0);
		}

		LeaveCriticalSection(&pModuleFileDownload->m_WriteLock);

		switch (pPacket->m_PacketHead.wCommandId) {
		case FILE_DOWNLOAD_DATA:
			theApp.m_Server.SendPacket(pSocketClient, FILE_DOWNLOAD_DATA, NULL, 0);
			break;
		case FILE_DOWNLOAD_DATA_TAIL:
			theApp.m_Server.SendPacket(pSocketClient, FILE_DOWNLOAD_DATA_TAIL, NULL, 0);
			break;
		}

		break;
	}

	// 收到了FILE_DOWNLOAD_CLOSE封包
	case WAIT_OBJECT_0 + 1:

		break;
	}

	delete pPacket;
}
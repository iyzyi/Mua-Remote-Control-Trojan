#include "pch.h"
#include "ModuleFileUpload.h"


CFileUpload::CFileUpload(CSocketClient* pClient) : CModule(pClient) {

}


CFileUpload::~CFileUpload() {

}


// 重写虚函数
void CFileUpload::OnRecvChildSocketClientPacket(CPacket* pPacket) {
	CSocketClient* pSocketClient = pPacket->m_pSocketClient;

	switch (pPacket->m_PacketHead.wCommandId) {

	case FILE_UPLOAD_DATA:
		break;
	case FILE_UPLOAD_DATA_TAIL:
		break;
	case FILE_UPLOAD_CLOSE:
		break;
	}
}


//
//void CFileUpload::UploadFile(LPWSTR pszFilePath) {
//	/*WCHAR pszFile[MAX_PATH] = L"C:\\Users\\iyzyi\\Desktop\\测试文件传输\\server\\发送\\测试文件传输，可删.7z";
//	if (!PathFileExists(pszFile)) {
//		MessageBox(L"文件不存在");
//		return;
//	}
//
//	HANDLE hFile = CreateFile(pszFile, FILE_READ_EA, FILE_SHARE_READ, 0, OPEN_EXISTING, 0, 0);
//	if (hFile == INVALID_HANDLE_VALUE)
//	{
//		MessageBox(L"文件句柄打开失败");
//		return;
//	}*/
//}
// ModuleFileTransfer.cpp: 实现文件
//

#include "pch.h"
#include "ModuleFileTransfer.h"
#include "afxdialogex.h"
#include "resource.h"
#include "pathcch.h"
#pragma comment(lib, "Pathcch.lib")
#include "ModuleFileUpload.h"
#include "ModuleFileDownload.h"

// CModuleFileTransfer 对话框

IMPLEMENT_DYNAMIC(CModuleFileTransfer, CDialogEx)

CModuleFileTransfer::CModuleFileTransfer(CWnd* pParent /*=nullptr*/, CClient * pClient/* = nullptr*/)
	: CDialogEx(IDD_DIALOG1, pParent)
{
	m_pClient = pClient;
}

CModuleFileTransfer::~CModuleFileTransfer()
{
}

void CModuleFileTransfer::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_EDIT2, m_EditDownloadLocalPath);
	DDX_Control(pDX, IDC_EDIT3, m_EditUploadLocalPath);
	DDX_Control(pDX, IDC_EDIT5, m_EditUploadRemotePath);
	DDX_Control(pDX, IDC_EDIT4, m_EditDownloadRemotePath);
}


BEGIN_MESSAGE_MAP(CModuleFileTransfer, CDialogEx)
	ON_BN_CLICKED(IDC_BUTTON5, &CModuleFileTransfer::OnBnClickedButton5)
	ON_BN_CLICKED(IDC_BUTTON6, &CModuleFileTransfer::OnBnClickedButton6)
	ON_BN_CLICKED(IDC_BUTTON3, &CModuleFileTransfer::OnBnClickedUpload)
	ON_BN_CLICKED(IDC_BUTTON4, &CModuleFileTransfer::OnBnClickedDownload)
END_MESSAGE_MAP()


// CModuleFileTransfer 消息处理程序

// 文件下载中的选择本地目录
void CModuleFileTransfer::OnBnClickedButton5()
{
	TCHAR				lpszPath[MAX_PATH];
	CString				strFolder = TEXT("");
	BROWSEINFO			sInfo;
	LPITEMIDLIST		lpidlBrowse;

	::ZeroMemory(&sInfo, sizeof(BROWSEINFO));
	sInfo.pidlRoot = 0;
	sInfo.pszDisplayName = lpszPath;
	sInfo.lpszTitle = _T("请选择保存目录");
	sInfo.ulFlags = BIF_DONTGOBELOWDOMAIN | BIF_RETURNONLYFSDIRS | BIF_NEWDIALOGSTYLE | BIF_EDITBOX;
	sInfo.lpfn = NULL;
	sInfo.hwndOwner = m_hWnd;

	// 显示文件夹选择对话框
	lpidlBrowse = ::SHBrowseForFolder(&sInfo);
	if (lpidlBrowse != NULL)
	{
		// 取得文件夹名
		if (::SHGetPathFromIDList(lpidlBrowse, lpszPath))
		{
			strFolder = lpszPath;
		}
	}
	if (lpidlBrowse != NULL)
	{
		::CoTaskMemFree(lpidlBrowse);
	}

	//MessageBox(strFolder);
	m_EditDownloadLocalPath.SetWindowText(strFolder);
}


// 上传文件中的选择本地文件
void CModuleFileTransfer::OnBnClickedButton6()
{
	CString strFile = _T("");

	//CFileDialog    dlgFile(TRUE, NULL, NULL, OFN_HIDEREADONLY, _T("Describe Files (*.cfg)|*.cfg|All Files (*.*)|*.*||"), NULL);
	CFileDialog    dlgFile(TRUE, NULL, NULL, OFN_HIDEREADONLY, _T("All Files (*.*)|*.*||"), NULL);

	if (dlgFile.DoModal())
	{
		strFile = dlgFile.GetPathName();
	}

	m_EditUploadLocalPath.SetWindowText(strFile);
}


// 点击上传
void CModuleFileTransfer::OnBnClickedUpload()
{
	CString strLocalPath = L"";
	CString strRemotePath = L"";

	if (m_EditUploadLocalPath.GetWindowTextLength() > MAX_PATH) {
		AfxMessageBox(L"本地路径过长");
		return;
	}
	if (m_EditUploadLocalPath.GetWindowTextLength() == 0) {
		AfxMessageBox(L"本地路径为空");
		return;
	}
	// 获取本地路径
	m_EditUploadLocalPath.GetWindowText(strLocalPath);
	// 从本地路径中分离出文件名
	CString strFileName;
	int n = strLocalPath.GetLength() - strLocalPath.ReverseFind('\\') - 1;
	strFileName = strLocalPath.Right(n);
	if (strFileName.GetLength() == 0) {
		AfxMessageBox(L"未能从本地路径中分离出文件名，请检查本地路径的格式是否正确。");
		return;
	}

	if (m_EditUploadRemotePath.GetWindowTextLength() > MAX_PATH) {
		AfxMessageBox(L"远程目录过长");
		return;
	}
	if (m_EditUploadRemotePath.GetWindowTextLength() == 0) {
		AfxMessageBox(L"远程目录为空");
		return;
	}

	// 获取远程目录
	m_EditUploadRemotePath .GetWindowText(strRemotePath);
	WCHAR pszRemotePathTemp[MAX_PATH];
	wcscpy_s(pszRemotePathTemp, strRemotePath.GetBuffer());
	strRemotePath.ReleaseBuffer();


	// 远程目录+文件名拼接成远程路径
	HRESULT Ret = PathCchAppend(pszRemotePathTemp, MAX_PATH, strFileName.AllocSysString());

	if (Ret != S_OK) {
		AfxMessageBox(L"路径拼接出错，请检查");
		return;
	}
	strRemotePath = CString(pszRemotePathTemp);

	if (strRemotePath.GetLength() >= MAX_PATH) {
		AfxMessageBox(L"拼接后的远程路径过长");
		return;
	}

	UploadFile(m_pClient, strLocalPath.GetBuffer(), strRemotePath.GetBuffer());
	strLocalPath.ReleaseBuffer();
	strRemotePath.ReleaseBuffer();
}


// 点击下载
void CModuleFileTransfer::OnBnClickedDownload()
{
	CString strRemotePath = L"";
	CString strLocalPath = L"";

	if (m_EditDownloadRemotePath.GetWindowTextLength() > MAX_PATH) {
		AfxMessageBox(L"远程路径过长");
		return;
	}
	if (m_EditDownloadRemotePath.GetWindowTextLength() == 0) {
		AfxMessageBox(L"远程路径为空");
		return;
	}
	// 获取远程路径
	m_EditDownloadRemotePath.GetWindowText(strRemotePath);
	// 从远程路径中分离出文件名
	CString strFileName;
	int n = strRemotePath.GetLength() - strRemotePath.ReverseFind('\\') - 1;
	strFileName = strRemotePath.Right(n);
	if (strFileName.GetLength() == 0) {
		AfxMessageBox(L"未能从远程路径中分离出文件名，请检查远程路径的格式是否正确。");
		return;
	}

	if (m_EditDownloadLocalPath.GetWindowTextLength() > MAX_PATH) {
		AfxMessageBox(L"本地目录过长");
		return;
	}
	if (m_EditDownloadLocalPath.GetWindowTextLength() == 0) {
		AfxMessageBox(L"本地目录为空");
		return;
	}

	// 获取本地目录
	m_EditDownloadLocalPath.GetWindowText(strLocalPath);
	WCHAR pszLocalPathTemp[MAX_PATH];
	wcscpy_s(pszLocalPathTemp, strLocalPath.GetBuffer());
	strLocalPath.ReleaseBuffer();


	// 本地目录+文件名拼接成本地路径
	HRESULT Ret = PathCchAppend(pszLocalPathTemp, MAX_PATH, strFileName.AllocSysString());

	if (Ret != S_OK) {
		AfxMessageBox(L"路径拼接出错，请检查");
		return;
	}
	strLocalPath = CString(pszLocalPathTemp);

	if (strLocalPath.GetLength() >= MAX_PATH) {
		AfxMessageBox(L"拼接后的本地路径过长");
		return;
	}

	DownloadFile(m_pClient, strRemotePath.GetBuffer(), strLocalPath.GetBuffer());
	strRemotePath.ReleaseBuffer();
	strLocalPath.ReleaseBuffer();
}


void CModuleFileTransfer::OnOK()
{
	//什么也不写
}

//然后重载PreTranslateMessage函数  
//把ESC键的消息，用RETURN键的消息替换，这样，按ESC的时候，也会执行刚才的OnOK函数  
BOOL CModuleFileTransfer::PreTranslateMessage(MSG* pMsg)
{
	if (pMsg->message == WM_KEYDOWN && pMsg->wParam == VK_ESCAPE)
	{
		pMsg->wParam = VK_RETURN;   //将ESC键的消息替换为回车键的消息，这样，按ESC的时候  
									//也会去调用OnOK函数，而OnOK什么也不做，这样ESC也被屏蔽  
	}
	return   CDialog::PreTranslateMessage(pMsg);
}
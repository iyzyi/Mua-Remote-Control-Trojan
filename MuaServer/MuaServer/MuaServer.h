// MuaServer.h: PROJECT_NAME 应用程序的主头文件
//

#pragma once

#ifndef __AFXWIN_H__
	#error "include 'pch.h' before including this file for PCH"
#endif

#include "resource.h"		// 主符号

#include "SocketServer.h"

// CMFCApplication4App:
// 有关此类的实现，请参阅 MFCApplication4.cpp
//

class CMFCApplication4App : public CWinApp
{
public:
	CSocketServer			m_Server;			// CSocketServer继承了CTcpPackServerPtr

public:
	CMFCApplication4App();
	~CMFCApplication4App();

// 重写
public:
	virtual BOOL InitInstance();

// 实现

	DECLARE_MESSAGE_MAP()

};

extern CMFCApplication4App theApp;


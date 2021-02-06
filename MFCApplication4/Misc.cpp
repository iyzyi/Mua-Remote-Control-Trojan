#include "pch.h"
#include "misc.h"



VOID RandomBytes(LPBYTE pbData, DWORD dwDataLen) {
	std::random_device rd;						// 产生一个 std::random_device 对象 rd
	std::default_random_engine random(rd());	// 用 rd 初始化一个随机数发生器 random

	for (DWORD i = 0; i < dwDataLen; i += 4) {
		DWORD dwRandNum = random();
		for (DWORD j = 0; j < 4; j++) {
			if (i + j >= dwDataLen) {
				return;
			}
			pbData[i + j] = dwRandNum & 0xff;
			dwRandNum >>= 8;
		}
	}
}


VOID PrintBytes(LPBYTE pbPrintData, DWORD dwDataLen)
{
	for (DWORD dwCount = 0; dwCount < dwDataLen; dwCount++) {
		printf("0x%02x ", pbPrintData[dwCount]);
		if (0 == (dwCount + 1) % 0x10) {
			putchar('\n');
		}
	}
	putchar('\n');
}


VOID PrintChars(CHAR *pbPrintData, DWORD dwDataLen) {
	for (DWORD dwCount = 0; dwCount < dwDataLen; dwCount++) {
		printf("%c", pbPrintData[dwCount]);
	}
	putchar('\n');
}


BOOL IsLittleEndding() {
	int i = 1;
	char c = *(char *)&i;
	return 1 ? true : false;
}


// 从buffer中偏移dwPos处取出一个DWORD，暂时默认小端存储，以后再完善吧。
DWORD GetDwordFromBuffer(PBYTE pbData, DWORD dwPos) {
	PBYTE pbData2 = pbData + dwPos;
	return pbData2[0] + (pbData2[1] << 8) + (pbData2[2] << 16) + (pbData2[3] << 24);
}


WORD GetWordFromBuffer(PBYTE pbData, DWORD dwPos) {
	PBYTE pbData2 = pbData + dwPos;
	return pbData2[0] + (pbData2[1] << 8);
}

BYTE GetByteFromBuffer(PBYTE pbData, DWORD dwPos) {
	PBYTE pbData2 = pbData + dwPos;
	return pbData2[0];
}


// 返回拷贝好的地址
PBYTE CopyBuffer(PBYTE pbSrc, DWORD dwLength, DWORD dwPos) {
	PBYTE pbDest = (PBYTE)xmalloc(dwLength);
	if (pbDest == NULL) {
		return NULL;
	}
	memcpy(pbDest, pbSrc+dwPos, dwLength);
	return pbDest;
}
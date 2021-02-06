#include "pch.h"
#include "misc.h"
#include <random>


void RandomBytes(LPBYTE pbData, DWORD dwDataLen) {
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


void PrintBytes(LPBYTE pbPrintData, DWORD dwDataLen)
{
	for (DWORD dwCount = 0; dwCount < dwDataLen; dwCount++) {
		printf("0x%02x ", pbPrintData[dwCount]);
		if (0 == (dwCount + 1) % 0x10) {
			putchar('\n');
		}
	}
	putchar('\n');
}


void PrintChars(CHAR *pbPrintData, DWORD dwDataLen) {
	for (DWORD dwCount = 0; dwCount < dwDataLen; dwCount++) {
		printf("%c", pbPrintData[dwCount]);
	}
	putchar('\n');
}
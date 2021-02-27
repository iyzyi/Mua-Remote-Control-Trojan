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
		DebugPrint("0x%02x ", pbPrintData[dwCount]);
		if (0 == (dwCount + 1) % 0x10) {
			DebugPrint("\n");
		}
	}
	DebugPrint("\n");
}


VOID PrintChars(CHAR *pbPrintData, DWORD dwDataLen) {
	for (DWORD dwCount = 0; dwCount < dwDataLen; dwCount++) {
		DebugPrint("%c", pbPrintData[dwCount]);
	}
	DebugPrint("\n");
}


VOID PrintData(LPBYTE pbPrintData, DWORD dwDataLen)
{
	if (dwDataLen > 512) {
		DebugPrint("共%d字节，出于效率考虑，仅打印前512字节\n", dwDataLen);
		dwDataLen = 512;
	}

	DWORD dwRow = 0, dwColumn = 0;
	for (dwRow = 0; dwRow < dwDataLen / 16 + 1; dwRow++) {
		for (dwColumn = 0; (dwRow * 16 + dwColumn < dwDataLen) && (dwColumn < 16); dwColumn++) {
			DebugPrint("0x%02x ", pbPrintData[dwRow * 16 + dwColumn]);
		}

		if (dwColumn != 16) {
			while (dwColumn < 16) {
				DebugPrint("     ");
				dwColumn++;
			}
		}
		DebugPrint("\t");

		for (dwColumn = 0; (dwRow * 16 + dwColumn < dwDataLen) && (dwColumn < 16); dwColumn++) {
			DWORD dwIndex = dwRow * 16 + dwColumn;
			if (pbPrintData[dwIndex] >= 32 && pbPrintData[dwIndex] <= 126) {
				DebugPrint("%c", pbPrintData[dwIndex]);
			}
			else {
				DebugPrint(".");
			}
		}
		DebugPrint("\n");
	}
	DebugPrint("\n");
}




BOOL IsLittleEndding() {
	int i = 1;
	char c = *(char *)&i;
	return 1 ? true : false;
}


QWORD GetQwordFromBuffer(PBYTE pbData, DWORD dwPos) {
	PBYTE pbData2 = pbData + dwPos;
	return pbData2[0] + (pbData2[1] << 8) + (pbData2[2] << 16) + (pbData2[3] << 24) + (pbData2[4] << 32) + (pbData2[5] << 40) + (pbData2[6] << 48) + (pbData2[7] << 56);
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


VOID WriteQwordToBuffer(PBYTE pbData, QWORD qwNum, DWORD dwPos) {
	PBYTE pbData2 = pbData + dwPos;
	pbData2[0] = qwNum & 0xff;
	pbData2[1] = (qwNum >> 8) & 0xff;
	pbData2[2] = (qwNum >> 16) & 0xff;
	pbData2[3] = (qwNum >> 24) & 0xff;
	pbData2[4] = (qwNum >> 32) & 0xff;
	pbData2[5] = (qwNum >> 40) & 0xff;
	pbData2[6] = (qwNum >> 48) & 0xff;
	pbData2[7] = (qwNum >> 56) & 0xff;
}


VOID WriteDwordToBuffer(PBYTE pbData, DWORD dwNum, DWORD dwPos) {
	PBYTE pbData2 = pbData + dwPos;
	pbData2[0] = dwNum & 0xff;
	pbData2[1] = (dwNum >> 8) & 0xff;
	pbData2[2] = (dwNum >> 16) & 0xff;
	pbData2[3] = (dwNum >> 24) & 0xff;
}


VOID WriteWordToBuffer(PBYTE pbData, WORD wNum, DWORD dwPos) {
	PBYTE pbData2 = pbData + dwPos;
	pbData2[0] = wNum & 0xff;
	pbData2[1] = (wNum >> 8) & 0xff;
}


VOID WriteByteToBuffer(PBYTE pbData, BYTE byNum, DWORD dwPos) {
	PBYTE pbData2 = pbData + dwPos;
	pbData2[0] = byNum;
}


// 这段代码不能在Release版中出现
#ifdef _DEBUG
CHAR* EncodeString(CHAR plain[]) {
	CHAR key[] = "iyzyi@BXS";
	DWORD dwKeyLength = strlen(key);
	DWORD dwPlainLength = strlen(plain);

	// 得用BYTE来计算，用CHAR来进行移位运算，会有0x87>>4=-8
	BYTE* buffer = new BYTE[dwPlainLength];
	memcpy(buffer, plain, dwPlainLength);

	int j = 0;
	bool NeedBecomeZero = false;
	for (int i = 0; i < dwPlainLength; i++)
	{
		if (j == 1 && NeedBecomeZero)
		{
			j = 0;
			NeedBecomeZero = false;
		}
		if (j == dwKeyLength)
		{
			j = 0;
			NeedBecomeZero = true;
		}
		buffer[i] ^= key[j];
		j++;
	}


	CHAR temp = buffer[dwPlainLength - 1] >> 3;
	for (int i = dwPlainLength - 1; i > 0; i--)
	{
		buffer[i] = (buffer[i] << 5) | (buffer[i - 1] >> 3);
	}
	buffer[0] = (buffer[0] << 5) | temp;


	DWORD dwCipherLength = dwPlainLength * 2;
	CHAR* cipher = new CHAR[dwCipherLength + 1];
	memset(cipher, 0, dwCipherLength + 1);
	for (int i = 0; i < dwPlainLength; i++) {
		cipher[2 * i] = ((buffer[i] >> 4) >= 0 && (buffer[i] >> 4) <= 9) ? ((buffer[i] >> 4) + '0') : ((buffer[i] >> 4) - 10 + 'A');
		cipher[2 * i + 1] = ((buffer[i] & 0xf) >= 0 && (buffer[i] & 0xf) <= 9) ? ((buffer[i] & 0xf) + '0') : ((buffer[i] & 0xf) - 10 + 'A');
	}

	printf("%s 加密为 %s\n", plain, cipher);
	delete[] buffer;
	return cipher;
}
#endif


VOID DecodeString(CHAR str[]) {
	CHAR key[] = "iyzyi@BXS";
	DWORD dwKeyLength = strlen(key);
	DWORD dwCipherLength = strlen(str);

	assert(dwCipherLength % 2 == 0);
	DWORD dwPlainLength = dwCipherLength / 2;
	BYTE* buffer = new BYTE[dwPlainLength + 1];
	CHAR hex[] = "0123456789ABCDEF";
	memset(buffer, 0, dwPlainLength + 1);
	for (int i = 0; i < dwCipherLength; i += 2) {
		assert((str[i] >= '0' && str[i] <= '9') || (str[i] >= 'A' && str[i] <= 'F'));
		assert((str[i + 1] >= '0' && str[i + 1] <= '9') || (str[i + 1] >= 'A' && str[i + 1] <= 'F'));
		buffer[i / 2] = ((strchr(hex, str[i]) - hex) << 4) + (strchr(hex, str[i + 1]) - hex);
	}


	CHAR temp = buffer[0] << 3;
	for (int i = 0; i < dwPlainLength - 1; i++)
	{
		buffer[i] = (buffer[i] >> 5) | (buffer[i + 1] << 3);
	}
	buffer[dwPlainLength - 1] = (buffer[dwPlainLength - 1] >> 5) | temp;


	int j = 0;
	bool NeedBecomeZero = false;
	for (int i = 0; i < dwPlainLength; i++)
	{
		if (j == 1 && NeedBecomeZero)
		{
			j = 0;
			NeedBecomeZero = false;
		}
		if (j == dwKeyLength)
		{
			j = 0;
			NeedBecomeZero = true;
		}
		buffer[i] ^= key[j];
		j++;
	}

	memset(str, 0, dwCipherLength);
	memcpy(str, buffer, dwPlainLength);
	delete[] buffer;
}
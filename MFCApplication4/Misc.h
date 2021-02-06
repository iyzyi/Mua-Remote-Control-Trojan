#pragma once

VOID RandomBytes(LPBYTE pbData, DWORD dwDataLen);

VOID PrintBytes(LPBYTE pbPrintData, DWORD dwDataLen);

VOID PrintChars(CHAR *pbPrintData, DWORD dwDataLen);

BOOL IsLittleEndding();

DWORD GetDwordFromBuffer(PBYTE pbData, DWORD dwPos = 0);

WORD GetWordFromBuffer(PBYTE pbData, DWORD dwPos = 0);

BYTE GetByteFromBuffer(PBYTE pbData, DWORD dwPos = 0);
#pragma once

#include "windows.h"

int RsaKeyGen(int iKeySize, LPSTR lpszPublicKeyFileName, LPSTR lpszPrivateKeyFileName);

int RsaEncrypt(PBYTE pbPlaintext, DWORD dwPlaintextLength, PBYTE pbCiphertext, PDWORD pdwCiphertextLength, PBYTE pbKeyBuffer, DWORD dwKeyBufferSize);

int RsaDecrypt(PBYTE pbCiphertext, DWORD dwCiphertextLength, PBYTE pbPlaintext, PDWORD pdwPlaintextLength, PBYTE pbKeyBuffer, DWORD dwKeyBufferSize);
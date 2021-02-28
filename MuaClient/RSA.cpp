#include "pch.h"
#include <stdio.h>
#include <Windows.h>
#include <bcrypt.h>
#include <Wincrypt.h>
#include "libccrypto.h"
#include "RSA.h"


//iKeySize						密钥长度，2048位或4096位
//lpszPublicKeyFileName			生成的公钥保存在此文件中
//lpszPrivateKeyFileName		生成的私钥保存在此文件中
int RsaKeyGen(int iKeySize, LPSTR lpszPublicKeyFileName, LPSTR lpszPrivateKeyFileName) {
	BCRYPT_ALG_HANDLE hAlgo = NULL;
	BCRYPT_KEY_HANDLE hKey = NULL;
	NTSTATUS status;

	if (iKeySize != 2048 && iKeySize != 4096) {
		printf("The key size must be either 2048 or 4096!\n");
		return -1;
	}

	printf("Generating keys...\n");

	if (!NT_SUCCESS(status = BCryptOpenAlgorithmProvider(
		&hAlgo,					// _Out_  BCRYPT_ALG_HANDLE *phAlgorithm,
		BCRYPT_RSA_ALGORITHM,   // _In_   LPCWSTR pszAlgId,
		NULL,                   // _In_   LPCWSTR pszImplementation,
		0                       // _In_   DWORD dwFlags
	)))
	{
		printf("BCryptOpenAlgorithmProvider() error: %x\n", status);
		goto Cleanup;
	}

	if (!NT_SUCCESS(status = BCryptGenerateKeyPair(
		hAlgo,					// _Inout_  BCRYPT_ALG_HANDLE hAlgorithm,
		&hKey,					// _Out_    BCRYPT_KEY_HANDLE *phKey,
		iKeySize,				// _In_     ULONG dwLength,
		0						// _In_     ULONG dwFlags
	)))
	{
		printf("BCryptGenerateKeyPair() error: %x\n", status);
		goto Cleanup;
	}

	if (!NT_SUCCESS(status = BCryptFinalizeKeyPair(
		hKey,					// _Inout_  BCRYPT_KEY_HANDLE hKey,
		0						// _In_     ULONG dwFlags
	)))
	{
		printf("BCryptFinalizeKeyPair() error: %x\n", status);
		goto Cleanup;
	}

	RSAExportKey(hKey, BCRYPT_RSAPUBLIC_BLOB, lpszPublicKeyFileName);
	RSAExportKey(hKey, BCRYPT_RSAPRIVATE_BLOB, lpszPrivateKeyFileName);

	printf("Done!\n");

Cleanup:

	if (hKey != NULL) {
		BCryptDestroyKey(
			hKey				// _Inout_  BCRYPT_KEY_HANDLE hKey
		);
	}

	if (hAlgo != NULL) {
		BCryptCloseAlgorithmProvider(
			hAlgo,				// _Inout_  BCRYPT_ALG_HANDLE hAlgorithm,
			0					// _In_     ULONG dwFlags
		);
	}

	return 0;
}




int RsaEncrypt(PBYTE pbPlaintext, DWORD dwPlaintextLength, PBYTE pbCiphertext, PDWORD pdwCiphertextLength, PBYTE pbKeyBuffer, DWORD dwKeyBufferSize) {
	BCRYPT_ALG_HANDLE hAlgo = NULL;
	BCRYPT_KEY_HANDLE hKey = NULL;
	PBYTE pbCiphertextTemp = NULL;
	CCRYPT_STATUS status;

	// 导入RSA公钥
	if (CCRYPT_STATUS_SUCCESS != (status = RSAImportKeyFromBuffer(&hAlgo, &hKey, pbKeyBuffer, dwKeyBufferSize, BCRYPT_RSAPUBLIC_BLOB))) {
		printf("RSAImportKey error (public): %x\n", status);
		return -1;
	}

	RSAEncrypt(hKey, pbPlaintext, dwPlaintextLength, &pbCiphertextTemp, pdwCiphertextLength);
	memcpy(pbCiphertext, pbCiphertextTemp, *pdwCiphertextLength);

	if (pbCiphertextTemp != NULL)
	{
		HeapFree(GetProcessHeap(), 0, pbCiphertextTemp);
	}
	BCryptDestroyKey(hKey);
	BCryptCloseAlgorithmProvider(hAlgo, 0);

	return 0;
}



int RsaDecrypt(PBYTE pbCiphertext, DWORD dwCiphertextLength, PBYTE pbPlaintext, PDWORD pdwPlaintextLength, PBYTE pbKeyBuffer, DWORD dwKeyBufferSize) {
	BCRYPT_ALG_HANDLE hAlgo = NULL;
	BCRYPT_KEY_HANDLE hKey = NULL;
	PBYTE pbPlaintextTemp = NULL;
	CCRYPT_STATUS status;

	// 导入RSA私钥
	if (CCRYPT_STATUS_SUCCESS != (status = RSAImportKeyFromBuffer(&hAlgo, &hKey, pbKeyBuffer, dwKeyBufferSize, BCRYPT_RSAPRIVATE_BLOB))) {
		printf("RSAImportKey error (private): %x\n", status);
		return -1;
	}

	RSADecrypt(hKey, pbCiphertext, dwCiphertextLength, &pbPlaintextTemp, pdwPlaintextLength);
	memcpy(pbPlaintext, pbPlaintextTemp, *pdwPlaintextLength);

	if (pbPlaintextTemp != NULL) {
		HeapFree(GetProcessHeap(), 0, pbPlaintextTemp);
	}
	BCryptDestroyKey(hKey);
	BCryptCloseAlgorithmProvider(hAlgo, 0);

	return 0;
}
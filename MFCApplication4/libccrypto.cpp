/**
* RSA encryption library. Uses AES for symmetric key encryption, and RSA
* for securing the symmetric key.
*
* This library has not been vetted, use at your own risk!
*
* @author  Daniel Hong
*
* This program is licensed under the GNU GENERAL PUBLIC LICENSE Version 2.
* A LICENSE file should have accompanied this program.
*/
#include "pch.h"
#include <Windows.h>
#include <Bcrypt.h>
#include <Wincrypt.h>
#include "libccrypto.h"

#if _DEBUG
#include <stdio.h>

VOID DebugPrint(LPCSTR pszFile, LPCSTR pszFunc, DWORD dwLine, DWORD dwCode)
{
	printf("Error: 0x%x in %s at function '%s' on line %d\n", dwCode, pszFile, pszFunc, dwLine);
}
#endif

CCRYPT_STATUS CCryptCreateFile(PFILE_CRYPTO_INFO pCryptoInfo, LPCTSTR lpFileName, DWORD dwMode)
{
	CCRYPT_STATUS status = CCRYPT_STATUS_SUCCESS;
	DWORD dwAccess = GENERIC_READ;
	DWORD dwCreateMode = OPEN_EXISTING;

	// Initialize the structure
	pCryptoInfo->hAlgo = NULL;
	pCryptoInfo->hKey = NULL;
	pCryptoInfo->hFile = NULL;
	pCryptoInfo->pbAESKey = NULL;

	// If we are opening the file for write, we need to setup the AES encryption
	if (dwMode == CCRYPT_FILE_WRITE)
	{
		dwAccess = (GENERIC_READ | GENERIC_WRITE);
		dwCreateMode = CREATE_ALWAYS;

		// Allocate memory for the AES key
		// We'll be generating a random key below
		if (NULL == (pCryptoInfo->pbAESKey = (PBYTE)HeapAlloc(
			GetProcessHeap(), // _In_  HANDLE hHeap,
			HEAP_ZERO_MEMORY, // _In_  DWORD dwFlags,
			AES256_KEY_SIZE   // _In_  SIZE_T dwBytes
		)))
		{
			status = CCRYPT_STATUS_HEAP_FAIL;
#if _DEBUG
			DebugPrint(__FILE__, "HeapAlloc", __LINE__, status);
#endif
			goto Cleanup;
		}

		// Generate a random set of 32 bytes to use for the AES key
		if (!NT_SUCCESS(status = BCryptGenRandom(
			NULL,                   // _Inout_  BCRYPT_ALG_HANDLE hAlgorithm,
			pCryptoInfo->pbAESKey,  // _Inout_  PUCHAR pbBuffer,
			AES256_KEY_SIZE,        // _In_     ULONG cbBuffer,
			BCRYPT_USE_SYSTEM_PREFERRED_RNG // _In_     ULONG dwFlags
		)))
		{
#if _DEBUG
			DebugPrint(__FILE__, "BCryptGenRandom", __LINE__, status);
#endif
			goto Cleanup;
		}

		// Initiate the AES algorithm provider and creates a handle to the AES crypto key
		if (CCRYPT_STATUS_SUCCESS != (status = CCryptOpenAESAlgorithmProvider(pCryptoInfo)))
		{
			goto Cleanup;
		}
	}

	if (INVALID_HANDLE_VALUE == (pCryptoInfo->hFile = CreateFile(
		lpFileName,     // _In_      LPCTSTR lpFileName,
		dwAccess,       // _In_      DWORD dwDesiredAccess,
		0,              // _In_      DWORD dwShareMode,
		NULL,           // _In_opt_  LPSECURITY_ATTRIBUTES lpSecurityAttributes,
		dwCreateMode,   // _In_      DWORD dwCreationDisposition,
		FILE_ATTRIBUTE_NORMAL | FILE_FLAG_OVERLAPPED, // _In_      DWORD dwFlagsAndAttributes,
		NULL // _In_opt_  HANDLE hTemplateFile
	)))
	{
		status = GetLastError();
#if _DEBUG
		DebugPrint(__FILE__, "CreateFile", __LINE__, status);
#endif
		goto Cleanup;
	}

	// If we get to this point, everything is all good so we'll just return
	return CCRYPT_STATUS_SUCCESS;

	// Error cases will get here
	// We will clean up resources and return with appropriate status code
Cleanup:

	// On error, clean up resources aquired by the CRYPTO_FILE_INFO struct
	if (status != CCRYPT_STATUS_SUCCESS)
	{
		CCryptCloseHandle(pCryptoInfo);
	}

	return status;
} /* CCryptCreateFile */

BOOL CCryptReadFile(PFILE_CRYPTO_INFO pCryptoInfo, BCRYPT_KEY_HANDLE hKey, LPBYTE *pbBuffer, PDWORD pdwBytesRead)
{
	DWORD dwLastError = 0;
	DWORD dwFileSize = 0;
	DWORD dwResult = 0;
	DWORD dwCipherTextLen = 0;
	PBYTE pbFileBuffer = NULL;
	PBYTE pbCipherBuffer = NULL;
	BYTE bMagic[MAGIC_SIZE];
	BYTE bEncryptedKey[AES256_ENCRYPTED_KEY_SIZE];
	BYTE bHmac[HMAC_SIZE];      // HMAC from the file
	PBYTE pbHmacCalc = NULL;  // The calculated HMAC
	OVERLAPPED overLapped;

	// Make sure file handle that should have been created from CCryptCreateFile() is valid
	if (pCryptoInfo->hFile == NULL || pCryptoInfo->hFile == INVALID_HANDLE_VALUE)
	{
		dwLastError = CCRYPT_STATUS_INVALID_HANDLE;
		goto SetErrorAndReturn;
	}

	// Get the file size so that we can read into the entire file
	if (INVALID_FILE_SIZE == (dwFileSize = GetFileSize(pCryptoInfo->hFile, NULL)))
	{
		dwLastError = INVALID_FILE_SIZE;
#if _DEBUG
		DebugPrint(__FILE__, "GetFileSize", __LINE__, dwLastError);
#endif
		goto SetErrorAndReturn;
	}

	// Now allocate memory for a buffer to read in the file
	if (NULL == (pbFileBuffer = (PBYTE)HeapAlloc(
		GetProcessHeap(),
		HEAP_ZERO_MEMORY,
		dwFileSize)))
	{
		dwLastError = CCRYPT_STATUS_HEAP_FAIL;
#if _DEBUG
		DebugPrint(__FILE__, "HeapAlloc", __LINE__, dwLastError);
#endif
		goto Cleanup;
	}

	// Setup the over lapped structure
	ZeroMemory(&overLapped, sizeof(OVERLAPPED));
	overLapped.Offset = 0;
	overLapped.OffsetHigh = 0;

	if (!ReadFile(
		pCryptoInfo->hFile, // _In_         HANDLE hFile,
		pbFileBuffer,   // _Out_        LPVOID lpBuffer,
		dwFileSize,     // _In_         DWORD nNumberOfBytesToRead,
		&dwResult,      // _Out_opt_    LPDWORD lpNumberOfBytesRead,
		&overLapped     // _Inout_opt_  LPOVERLAPPED lpOverlapped
	))
	{
		// If not an IO pending error, exit function
		if (GetLastError() != ERROR_IO_PENDING)
		{
			dwLastError = CCRYPT_STATUS_USE_DEFAULT;
#if _DEBUG
			DebugPrint(__FILE__, "ReadFile", __LINE__, dwLastError);
#endif
			goto Cleanup;
		}
	}

	// Wait for the IO operation to complete
	GetOverlappedResult(
		pCryptoInfo->hFile, // _In_   HANDLE hFile,
		&overLapped, // _In_   LPOVERLAPPED lpOverlapped,
		&dwResult,   // _Out_  LPDWORD lpNumberOfBytesTransferred,
		TRUE         // _In_   BOOL bWait
	);

	// Now we need to parse out the various parts of the file:
	// Magic value
	if (!SubString(pbFileBuffer, 0, MAGIC_SIZE, bMagic))
	{
		dwLastError = CCRYPT_STATUS_USE_DEFAULT;
		goto Cleanup;
	}

	// Check if magic value is correct. This doesn't provide any security
	// it'll just give us the ability to bail out if the file wasn't "valid"
	if (!TestMagicValue(bMagic))
	{
		dwLastError = CCRYPT_STATUS_INVALID_FILE;
		goto Cleanup;
	}

	// Now we can continue...

	// Encrypted AES key
	if (!SubString(pbFileBuffer, MAGIC_SIZE, AES256_ENCRYPTED_KEY_SIZE, bEncryptedKey))
	{
		dwLastError = CCRYPT_STATUS_USE_DEFAULT;
		goto Cleanup;
	}

	// HMAC
	if (!SubString(pbFileBuffer, (MAGIC_SIZE + AES256_ENCRYPTED_KEY_SIZE), HMAC_SIZE, bHmac))
	{
		dwLastError = CCRYPT_STATUS_USE_DEFAULT;
		goto Cleanup;
	}

	// Determine length of the cipher text
	dwCipherTextLen = dwFileSize - (MAGIC_SIZE + AES256_ENCRYPTED_KEY_SIZE + HMAC_SIZE);

	// Allocate a buffer for the cipher text
	if (NULL == (pbCipherBuffer = (PBYTE)HeapAlloc(
		GetProcessHeap(),
		HEAP_ZERO_MEMORY,
		dwCipherTextLen)))
	{
		dwLastError = CCRYPT_STATUS_HEAP_FAIL;
#if _DEBUG
		DebugPrint(__FILE__, "HeapAlloc", __LINE__, dwLastError);
#endif
		goto Cleanup;
	}

	// Now we can get the cipher text substring from the original file buffer
	if (!SubString(
		pbFileBuffer,
		(MAGIC_SIZE + AES256_ENCRYPTED_KEY_SIZE + HMAC_SIZE),
		dwCipherTextLen,
		pbCipherBuffer
	))
	{
		dwLastError = CCRYPT_STATUS_USE_DEFAULT;
		goto Cleanup;
	}

	// Decrypt the AES key
	if (CCRYPT_STATUS_SUCCESS != (dwLastError = RSADecrypt(
		hKey,
		bEncryptedKey,
		AES256_ENCRYPTED_KEY_SIZE,
		&(pCryptoInfo->pbAESKey),
		NULL
	)))
	{
#if _DEBUG
		DebugPrint(__FILE__, "RSADecrypt", __LINE__, dwLastError);
#endif
		goto Cleanup;
	}

	// Calculate the HMAC based on cipher text from file
	if (CCRYPT_STATUS_SUCCESS != (dwLastError = CCryptHmac(
		pbCipherBuffer,
		dwCipherTextLen,
		&pbHmacCalc,
		(PCHAR)pCryptoInfo->pbAESKey,
		AES256_KEY_SIZE
	)))
	{
#if _DEBUG
		DebugPrint(__FILE__, "CCryptHmac", __LINE__, dwLastError);
#endif
		goto Cleanup;
	}

	// Now we need to test the HMAC to ensure that the cipher text has not been modified
	if (!ByteCompare(bHmac, HMAC_SIZE, pbHmacCalc, HMAC_SIZE))
	{
		dwLastError = CCRYPT_STATUS_INVALID_HMAC;
		goto Cleanup;
	}

	// At this point, we can assume that the cipher text has not 
	// been modified, so we will now decrypt it

	// Get AES crypto provider
	if (CCRYPT_STATUS_SUCCESS != (dwLastError = CCryptOpenAESAlgorithmProvider(pCryptoInfo)))
	{
#if _DEBUG
		DebugPrint(__FILE__, "CCryptOpenAESAlgorithmProvider", __LINE__, dwLastError);
#endif
		goto Cleanup;
	}

	// Get size of plain text
	if (!NT_SUCCESS(dwLastError = BCryptDecrypt(
		pCryptoInfo->hKey,  // _Inout_      BCRYPT_KEY_HANDLE hKey,
		pbCipherBuffer,     // _In_         PUCHAR pbInput,
		dwCipherTextLen,    // _In_         ULONG cbInput,
		NULL,   // _In_opt_     VOID *pPaddingInfo,
		NULL,   // _Inout_opt_  PUCHAR pbIV,
		0,      // _In_         ULONG cbIV,
		NULL,   // _Out_opt_    PUCHAR pbOutput,
		0,      // _In_         ULONG cbOutput,
		&dwResult, // _Out_        ULONG *pcbResult,
		BCRYPT_BLOCK_PADDING // _In_         ULONG dwFlags
	)))
	{
#if _DEBUG
		DebugPrint(__FILE__, "BCryptDecrypt", __LINE__, dwLastError);
#endif
		goto Cleanup;
	}

	// Allocate memory for the plain text buffer
	if (NULL == (*pbBuffer = (PBYTE)HeapAlloc(
		GetProcessHeap(),
		HEAP_ZERO_MEMORY,
		dwResult)))
	{
		dwLastError = CCRYPT_STATUS_HEAP_FAIL;
#if _DEBUG
		DebugPrint(__FILE__, "HeapAlloc", __LINE__, dwLastError);
#endif
		goto Cleanup;
	}

	// Now we can decrypt!
	if (!NT_SUCCESS(dwLastError = BCryptDecrypt(
		pCryptoInfo->hKey, // _Inout_      BCRYPT_KEY_HANDLE hKey,
		pbCipherBuffer, // _In_         PUCHAR pbInput,
		dwCipherTextLen, // _In_         ULONG cbInput,
		NULL, //_In_opt_     VOID *pPaddingInfo,
		NULL, //_Inout_opt_  PUCHAR pbIV,
		0, //_In_         ULONG cbIV,
		*pbBuffer, // _Out_opt_    PUCHAR pbOutput,
		dwResult, // _In_         ULONG cbOutput,
		pdwBytesRead, // _Out_        ULONG *pcbResult,
		BCRYPT_BLOCK_PADDING // _In_         ULONG dwFlags
	)))
	{
#if _DEBUG
		DebugPrint(__FILE__, "BCryptDecrypt", __LINE__, dwLastError);
#endif
		goto Cleanup;
	}

Cleanup:

	if (pbFileBuffer != NULL)
	{
		HeapFree(GetProcessHeap(), 0, pbFileBuffer);
	}
	if (pbCipherBuffer != NULL)
	{
		HeapFree(GetProcessHeap(), 0, pbCipherBuffer);
	}
	if (pbHmacCalc != NULL)
	{
		HeapFree(GetProcessHeap(), 0, pbHmacCalc);
	}

	// On error, clean up resources aquired by the CRYPTO_FILE_INFO struct
	if (dwLastError != CCRYPT_STATUS_SUCCESS)
	{
		CCryptCloseHandle(pCryptoInfo);
	}

SetErrorAndReturn:

	// Don't set an error code if the function that errored out already set on
	if (dwLastError != CCRYPT_STATUS_USE_DEFAULT)
	{
		SetLastError(dwLastError);
	}
	return (dwLastError == 0);
} /* CCryptReadFile */

BOOL CCryptWriteFile(PFILE_CRYPTO_INFO pCryptoInfo, LPBYTE pbBuffer, DWORD dwBytesToWrite, PDWORD pdwBytesWritten)
{
	DWORD dwLastError = 0;
	DWORD dwFileSize = 0;
	DWORD dwPlainTextSize = 0;
	DWORD dwBytesRead = 0;
	DWORD dwResult = 0;
	BYTE bMagic[MAGIC_SIZE];
	PBYTE pbCipherBuffer = NULL;
	PBYTE pbPlainBuffer = NULL;
	NTSTATUS status = CCRYPT_STATUS_SUCCESS;
	OVERLAPPED overLapped;

	// Make sure file handle that should have been created from CCryptCreateFile() is valid
	if (pCryptoInfo->hFile == NULL || pCryptoInfo->hFile == INVALID_HANDLE_VALUE)
	{
		dwLastError = CCRYPT_STATUS_INVALID_HANDLE;
		goto SetErrorAndReturn;
	}

	// Set over lapped
	ZeroMemory(&overLapped, sizeof(OVERLAPPED));
	overLapped.Offset = 0;
	overLapped.OffsetHigh = 0;

	// First we will make sure that the file hasn't been finalized
	// We'll check this by looking for our "magic" value
	// at bytes 1-4
	if (!ReadFile(
		pCryptoInfo->hFile, // _In_         HANDLE hFile,
		bMagic,    // _Out_        LPVOID lpBuffer,
		MAGIC_SIZE,    // _In_         DWORD nNumberOfBytesToRead,
		&dwBytesRead,  // _Out_opt_    LPDWORD lpNumberOfBytesRead,
		&overLapped    // _Inout_opt_  LPOVERLAPPED lpOverlapped
	))
	{
		dwLastError = GetLastError();

		// If the error is not because of pending IO, then we will
		// set the error and return, otherwise we'll continue below
		// and wait for the IO operation to finish
		if (dwLastError != ERROR_IO_PENDING)
		{
#if _DEBUG
			DebugPrint(__FILE__, "ReadFile", __LINE__, dwLastError);
#endif
			goto SetErrorAndReturn;
		}
	}

	// Wait for IO operation to end
	GetOverlappedResult(
		pCryptoInfo->hFile, // _In_   HANDLE hFile,
		&overLapped,        // _In_   LPOVERLAPPED lpOverlapped,
		&dwResult,  // _Out_  LPDWORD lpNumberOfBytesTransferred,
		TRUE        // _In_   BOOL bWait
	);

	// Test for our magic values
	// If magic value appear in the file, we can assume that the file
	// has been finalized, i.e. the key has already been encrypted with RSA
	if (dwBytesRead != 0 && !TestMagicValue(bMagic))
	{
		dwLastError = CCRYPT_STATUS_FILE_FINALIZED;
		goto SetErrorAndReturn;
	}

	// Cleanup
	ZeroMemory(bMagic, MAGIC_SIZE);

	// Get size of the file, we'll use this value to read in the entire contents of the file
	// as well as for allocating memory for the new write buffer
	if (INVALID_FILE_SIZE == (dwFileSize = GetFileSize(pCryptoInfo->hFile, NULL)))
	{
		dwLastError = INVALID_FILE_SIZE;
#if _DEBUG
		DebugPrint(__FILE__, "GetFileSize", __LINE__, dwLastError);
#endif
		goto SetErrorAndReturn;
	}

	// If the file was empty, we just need to allocate a buffer for the file write
	if (dwFileSize == 0)
	{
		if (NULL == (pbPlainBuffer = (PBYTE)HeapAlloc(
			GetProcessHeap(),
			HEAP_ZERO_MEMORY,
			dwBytesToWrite)))
		{
			dwLastError = CCRYPT_STATUS_HEAP_FAIL;
#if _DEBUG
			DebugPrint(__FILE__, "HeapAlloc", __LINE__, dwLastError);
#endif
			goto Cleanup;
		}
	}
	// Otherwise, we will need to decrypt the file first
	else
	{
		// Now we can allocate the memory for the file read
		if (NULL == (pbCipherBuffer = (PBYTE)HeapAlloc(
			GetProcessHeap(),
			HEAP_ZERO_MEMORY,
			dwFileSize)))
		{
			dwLastError = CCRYPT_STATUS_HEAP_FAIL;
#if _DEBUG
			DebugPrint(__FILE__, "HeapAlloc", __LINE__, dwLastError);
#endif
			goto Cleanup;
		}

		// Finally we can read the file!
		if (!ReadFile(
			pCryptoInfo->hFile, // _In_         HANDLE hFile,
			pbCipherBuffer, // _Out_        LPVOID lpBuffer,
			dwFileSize,     // _In_         DWORD nNumberOfBytesToRead,
			&dwBytesRead,   // _Out_opt_    LPDWORD lpNumberOfBytesRead,
			&overLapped     // _Inout_opt_  LPOVERLAPPED lpOverlapped
		))
		{
			dwLastError = GetLastError();

			// If the error is not because of pending IO, then we will
			// set the error and return, otherwise we'll continue below
			// and wait for the IO operation to finish
			if (dwLastError != ERROR_IO_PENDING)
			{
#if _DEBUG
				DebugPrint(__FILE__, "ReadFile", __LINE__, dwLastError);
#endif
				goto Cleanup;
			}
		}

		// Wait for pending IO operation to end
		GetOverlappedResult(
			pCryptoInfo->hFile, // _In_   HANDLE hFile,
			&overLapped,        // _In_   LPOVERLAPPED lpOverlapped,
			&dwResult,  // _Out_  LPDWORD lpNumberOfBytesTransferred,
			TRUE        // _In_   BOOL bWait
		);

		// Get size of plain text needed for memory allocation for decryption
		if (!NT_SUCCESS(status = BCryptDecrypt(
			pCryptoInfo->hKey,  // _Inout_      BCRYPT_KEY_HANDLE hKey,
			pbCipherBuffer,     // _In_         PUCHAR pbInput,
			dwFileSize,         // _In_         ULONG cbInput,
			NULL,       // _In_opt_     VOID *pPaddingInfo,
			NULL,       // _Inout_opt_  PUCHAR pbIV,
			0,          // _In_         ULONG cbIV,
			NULL,       // _Out_opt_    PUCHAR pbOutput,
			0,          // _In_         ULONG cbOutput,
			&dwPlainTextSize,    // _Out_        ULONG *pcbResult,
			BCRYPT_BLOCK_PADDING // _In_         ULONG dwFlags
		)))
		{
			dwLastError = status;
#if _DEBUG
			DebugPrint(__FILE__, "BCryptDecrypt", __LINE__, dwLastError);
#endif
			goto Cleanup;
		}

		// We are just allocating enough to decrypt the contents of the file.
		// We'll realloc later to append the new buffer with current one
		if (NULL == (pbPlainBuffer = (PBYTE)HeapAlloc(
			GetProcessHeap(),
			HEAP_ZERO_MEMORY,
			dwPlainTextSize + dwBytesToWrite)))
		{
			dwLastError = CCRYPT_STATUS_HEAP_FAIL;
#if _DEBUG
			DebugPrint(__FILE__, "HeapAlloc", __LINE__, dwLastError);
#endif
			goto Cleanup;
		}

		// Now we can actually decrypt the cipher text
		if (!NT_SUCCESS(status = BCryptDecrypt(
			pCryptoInfo->hKey,  // _Inout_      BCRYPT_KEY_HANDLE hKey,
			pbCipherBuffer,     // _In_         PUCHAR pbInput,
			dwFileSize,         // _In_         ULONG cbInput,
			NULL,               // _In_opt_     VOID *pPaddingInfo,
			NULL,               // _Inout_opt_  PUCHAR pbIV,
			0,                  // _In_         ULONG cbIV,
			pbPlainBuffer,      // _Out_opt_    PUCHAR pbOutput,
			dwPlainTextSize,    // _In_         ULONG cbOutput,
			&dwPlainTextSize,   // _Out_        ULONG *pcbResult,
			BCRYPT_BLOCK_PADDING // _In_         ULONG dwFlags
		)))
		{
			dwLastError = status;
#if _DEBUG
			DebugPrint(__FILE__, "BCryptDecrypt", __LINE__, dwLastError);
#endif
			goto Cleanup;
		}

		// Free up the buffer we used for the cipher text earlier
		// We are going to reuse it
		HeapFree(GetProcessHeap(), 0, pbCipherBuffer);
	}

	// Copy data buffer into our file write buffer
	CopyMemory(
		(pbPlainBuffer + dwPlainTextSize),  // _In_  PVOID Destination,
		pbBuffer,       // _In_  const VOID *Source,
		dwBytesToWrite  // _In_  SIZE_T Length
	);

	// Get size of encrypted buffer
	if (NT_SUCCESS(status = BCryptEncrypt(
		pCryptoInfo->hKey,  // _Inout_      BCRYPT_KEY_HANDLE hKey,
		pbPlainBuffer,      // _In_         PUCHAR pbInput,
		(dwPlainTextSize + dwBytesToWrite), // _In_         ULONG cbInput,
		NULL,           // _In_opt_     VOID *pPaddingInfo,
		NULL,           // _Inout_opt_  PUCHAR pbIV,
		0,              // _In_         ULONG cbIV,
		NULL,           // _Out_opt_    PUCHAR pbOutput,
		0,              // _In_         ULONG cbOutput,
		&dwFileSize,    // _Out_        ULONG *pcbResult,
		BCRYPT_BLOCK_PADDING // _In_         ULONG dwFlags
	)))
	{
		// Allocate memory buffer for the encrypted text
		// Now we can allocate the memory for the file read
		if (NULL == (pbCipherBuffer = (PBYTE)HeapAlloc(
			GetProcessHeap(),
			HEAP_ZERO_MEMORY,
			dwFileSize)))
		{
			dwLastError = CCRYPT_STATUS_HEAP_FAIL;
#if _DEBUG
			DebugPrint(__FILE__, "HeapAlloc", __LINE__, dwLastError);
#endif
			goto Cleanup;
		}

		// Now we can actual encrypt the plain text
		if (!NT_SUCCESS(status = BCryptEncrypt(
			pCryptoInfo->hKey,  // _Inout_      BCRYPT_KEY_HANDLE hKey,
			pbPlainBuffer,      // _In_         PUCHAR pbInput,
			(dwPlainTextSize + dwBytesToWrite), // _In_         ULONG cbInput,
			NULL,           // _In_opt_     VOID *pPaddingInfo,
			NULL,           // _Inout_opt_  PUCHAR pbIV,
			0,              // _In_         ULONG cbIV,
			pbCipherBuffer, // _Out_opt_    PUCHAR pbOutput,
			dwFileSize,     // _In_         ULONG cbOutput,
			&dwFileSize,    // _Out_        ULONG *pcbResult,
			BCRYPT_BLOCK_PADDING // _In_         ULONG dwFlags
		)))
		{
			dwLastError = status;
#if _DEBUG
			DebugPrint(__FILE__, "BCryptEncrypt", __LINE__, dwLastError);
#endif
			goto Cleanup;
		}

		// Now we can write the encrypted text to file
		if (!WriteFile(
			pCryptoInfo->hFile, // _In_         HANDLE hFile,
			pbCipherBuffer,      // _In_         LPCVOID lpBuffer,
			dwFileSize,          // _In_         DWORD nNumberOfBytesToWrite,
			pdwBytesWritten,     // _Out_opt_    LPDWORD lpNumberOfBytesWritten,
			&overLapped          // _Inout_opt_  LPOVERLAPPED lpOverlapped
		))
		{
			// If the error is not because of pending IO, then we will
			// set the error and return, otherwise we'll continue below
			// and wait for the IO operation to finish
			if (GetLastError() != ERROR_IO_PENDING)
			{
#if _DEBUG
				DebugPrint(__FILE__, "WriteFile", __LINE__, GetLastError());
#endif
				dwLastError = CCRYPT_STATUS_USE_DEFAULT;
				goto Cleanup;
			}
		}

		// Wait for pending IO operation to end
		GetOverlappedResult(
			pCryptoInfo->hFile, // _In_   HANDLE hFile,
			&overLapped, // _In_   LPOVERLAPPED lpOverlapped,
			&dwResult, // _Out_  LPDWORD lpNumberOfBytesTransferred,
			TRUE // _In_   BOOL bWait
		);

		// Reset the return status
		dwLastError = CCRYPT_STATUS_SUCCESS;
	}
	else
	{
		// If we couldn't get the size of the encrypted text, bail out
		dwLastError = status;
#if _DEBUG
		DebugPrint(__FILE__, "WriteFile", __LINE__, dwLastError);
#endif
	}

Cleanup:

	if (pbCipherBuffer != NULL)
	{
		HeapFree(GetProcessHeap(), 0, pbCipherBuffer);
	}
	if (pbPlainBuffer != NULL)
	{
		HeapFree(GetProcessHeap(), 0, pbPlainBuffer);
	}

	// On error, clean up resources aquired by the CRYPTO_FILE_INFO struct
	if (dwLastError != CCRYPT_STATUS_SUCCESS)
	{
		CCryptCloseHandle(pCryptoInfo);
	}

SetErrorAndReturn:

	// Don't set an error code if the function that errored out already set on
	if (dwLastError != CCRYPT_STATUS_USE_DEFAULT)
	{
		SetLastError(dwLastError);
	}
	return (dwLastError == 0);
} /* CCryptWriteFile */

BOOL CCryptFinalizeFile(PFILE_CRYPTO_INFO pCryptoInfo, BCRYPT_KEY_HANDLE hKey)
{
	DWORD dwFileSize;
	DWORD dwResult = 0;
	PBYTE pbFileBuffer = NULL;
	PBYTE pbEncryptedKey = NULL;
	PBYTE pbHmac = NULL;
	BYTE bMagic[MAGIC_SIZE] = { MAGIC_CHAR_1, MAGIC_CHAR_2, MAGIC_CHAR_2, MAGIC_CHAR_1 };
	DWORD dwEncryptedKeySize = 0;
	CCRYPT_STATUS status = CCRYPT_STATUS_SUCCESS;
	DWORD dwLastError = 0;
	OVERLAPPED overLapped;

	// Encrypt the AES key with the RSA public key that was imported
	RSAEncrypt(hKey, pCryptoInfo->pbAESKey, AES256_KEY_SIZE, &pbEncryptedKey, &dwEncryptedKeySize);

	// Get size of the file, we'll use this value to read in the entire contents of the file
	// as well as for allocating memory for the new buffer that will also include our
	// encrypted AES key
	if (INVALID_FILE_SIZE == (dwFileSize = GetFileSize(pCryptoInfo->hFile, NULL)))
	{
		dwLastError = INVALID_FILE_SIZE;
#if _DEBUG
		DebugPrint(__FILE__, "GetFileSize", __LINE__, dwLastError);
#endif
		goto SetErrorAndReturn;
	}

	// Allocate a buffer large enough to read in the entire file and 
	// account for the encrypted AES key, HMAC, and magic value
	if (NULL == (pbFileBuffer = (PBYTE)HeapAlloc(
		GetProcessHeap(),
		HEAP_ZERO_MEMORY,
		dwFileSize + MAGIC_SIZE + dwEncryptedKeySize + HMAC_SIZE)))
	{
		dwLastError = CCRYPT_STATUS_HEAP_FAIL;
#if _DEBUG
		DebugPrint(__FILE__, "HeapAlloc", __LINE__, dwLastError);
#endif
		goto Cleanup;
	}

	// Set over lapped
	ZeroMemory(&overLapped, sizeof(OVERLAPPED));
	overLapped.Offset = 0;
	overLapped.OffsetHigh = 0;

	// Read in the file
	// We are skipping ahead in the buffer so that we can put magic value,
	// encrypted AES key, and HMAC at the beginning
	if (!ReadFile(
		pCryptoInfo->hFile, // _In_         HANDLE hFile,
		(pbFileBuffer + MAGIC_SIZE + dwEncryptedKeySize + HMAC_SIZE), // _Out_        LPVOID lpBuffer,
		dwFileSize,  // _In_         DWORD nNumberOfBytesToRead,
		&dwResult,   // _Out_opt_    LPDWORD lpNumberOfBytesRead,
		&overLapped  // _Inout_opt_  LPOVERLAPPED lpOverlapped
	))
	{
		// If the error is not because of pending IO, then we will
		// set the error and return, otherwise we'll continue below
		// and wait for the IO operation to finish
		if (GetLastError() != ERROR_IO_PENDING) {
#if _DEBUG
			DebugPrint(__FILE__, "ReadFile", __LINE__, GetLastError());
#endif
			dwLastError = CCRYPT_STATUS_USE_DEFAULT;
			goto Cleanup;
		}
	}

	// Wait for the pending IO operation to complete
	GetOverlappedResult(
		pCryptoInfo->hFile, // _In_   HANDLE hFile,
		&overLapped,        // _In_   LPOVERLAPPED lpOverlapped,
		&dwResult,  // _Out_  LPDWORD lpNumberOfBytesTransferred,
		TRUE        // _In_   BOOL bWait
	);

	// Now we can HMAC the encrypted text
	// Skipping ahead in the buffer for same reasons as stated above
	if (CCRYPT_STATUS_SUCCESS != (status = CCryptHmac(
		(pbFileBuffer + MAGIC_SIZE + dwEncryptedKeySize + HMAC_SIZE),
		dwFileSize,
		&pbHmac,
		(PCHAR)pCryptoInfo->pbAESKey,
		AES256_KEY_SIZE
	)))
	{
		dwLastError = status;
#if _DEBUG
		DebugPrint(__FILE__, "CCryptHmac", __LINE__, dwLastError);
#endif
		goto SetErrorAndReturn;
	}

	// Copy the magic values into the buffer
	CopyMemory(
		pbFileBuffer, // _In_  PVOID Destination,
		bMagic,       // _In_  const VOID *Source,
		MAGIC_SIZE    // _In_  SIZE_T Length
	);

	// Copy the encrypted AES key into the buffer
	CopyMemory(
		(pbFileBuffer + MAGIC_SIZE),  // _In_  PVOID Destination,
		pbEncryptedKey,     // _In_  const VOID *Source,
		dwEncryptedKeySize  // _In_  SIZE_T Length
	);

	// Copy the HMAC into the buffer
	CopyMemory(
		(pbFileBuffer + MAGIC_SIZE + dwEncryptedKeySize),  // _In_  PVOID Destination,
		pbHmac,    // _In_  const VOID *Source,
		HMAC_SIZE  // _In_  SIZE_T Length
	);

	// Now that we've got the buffer in order, we can write it back out to the file
	if (!WriteFile(
		pCryptoInfo->hFile, // _In_         HANDLE hFile,
		pbFileBuffer,       // _In_         LPCVOID lpBuffer,
		(dwFileSize + MAGIC_SIZE + dwEncryptedKeySize + HMAC_SIZE), // _In_         DWORD nNumberOfBytesToWrite,
		&dwResult,   // _Out_opt_    LPDWORD lpNumberOfBytesWritten,
		&overLapped  // _Inout_opt_  LPOVERLAPPED lpOverlapped
	))
	{
		// If the error is not because of pending IO, then we will
		// set the error and return, otherwise we'll continue below
		// and wait for the IO operation to finish
		if (GetLastError() != ERROR_IO_PENDING) {
#if _DEBUG
			DebugPrint(__FILE__, "WriteFile", __LINE__, GetLastError());
#endif
			dwLastError = CCRYPT_STATUS_USE_DEFAULT;
			goto Cleanup;
		}
	}

	// Wait for IO operation to complete
	GetOverlappedResult(
		pCryptoInfo->hFile, // _In_   HANDLE hFile,
		&overLapped,        // _In_   LPOVERLAPPED lpOverlapped,
		&dwResult,  // _Out_  LPDWORD lpNumberOfBytesTransferred,
		TRUE        // _In_   BOOL bWait
	);

	// Reset error to success
	dwLastError = CCRYPT_STATUS_SUCCESS;

Cleanup:

	if (pbFileBuffer != NULL)
	{
		HeapFree(GetProcessHeap(), 0, pbFileBuffer);
	}
	if (pbEncryptedKey != NULL)
	{
		HeapFree(GetProcessHeap(), 0, pbEncryptedKey);
	}
	if (pbHmac != NULL)
	{
		HeapFree(GetProcessHeap(), 0, pbHmac);
	}

	// On error, clean up resources aquired by the CRYPTO_FILE_INFO struct
	if (dwLastError != CCRYPT_STATUS_SUCCESS)
	{
		CCryptCloseHandle(pCryptoInfo);
	}

SetErrorAndReturn:

	// Don't set an error code if the function that errored out already set it
	if (dwLastError != CCRYPT_STATUS_USE_DEFAULT)
	{
		SetLastError(dwLastError);
	}
	return (dwLastError == 0);
} /* CCryptFinalizeFile */

void CCryptCloseHandle(PFILE_CRYPTO_INFO pCryptoInfo)
{
	if (pCryptoInfo->pbAESKey != NULL)
	{
		HeapFree(GetProcessHeap(), 0, pCryptoInfo->pbAESKey);
	}
	if (pCryptoInfo->hKey != NULL)
	{
		BCryptDestroyKey(pCryptoInfo->hKey);
	}
	if (pCryptoInfo->hAlgo != NULL)
	{
		BCryptCloseAlgorithmProvider(pCryptoInfo->hAlgo, 0);
	}
	if (pCryptoInfo->hFile != NULL)
	{
		CloseHandle(pCryptoInfo->hFile);
	}
} /* CCryptCloseHandle */

CCRYPT_STATUS CCryptOpenAESAlgorithmProvider(PFILE_CRYPTO_INFO pCryptoInfo)
{
	CCRYPT_STATUS status = CCRYPT_STATUS_SUCCESS;

	// Initiate a algorithm provider for AES encryption
	if (!NT_SUCCESS(status = BCryptOpenAlgorithmProvider(
		&(pCryptoInfo->hAlgo),  // _Out_  BCRYPT_ALG_HANDLE *phAlgorithm,
		BCRYPT_AES_ALGORITHM,   // _In_   LPCWSTR pszAlgId,
		NULL,                   // _In_   LPCWSTR pszImplementation,
		0                       // _In_   DWORD dwFlags
	)))
	{
#if _DEBUG
		DebugPrint(__FILE__, "BCryptOpenAlgorithmProvider", __LINE__, status);
#endif
		return status;
	}

	// Use CBC chaining
	if (!NT_SUCCESS(status = BCryptSetProperty(
		pCryptoInfo->hAlgo,             // _Inout_  BCRYPT_HANDLE hObject,
		BCRYPT_CHAINING_MODE,           // _In_     LPCWSTR pszProperty,
		(PUCHAR)BCRYPT_CHAIN_MODE_CBC,   // _In_     PUCHAR pbInput,
		sizeof(BCRYPT_CHAIN_MODE_CBC),  // _In_     ULONG cbInput,
		0                               // _In_     ULONG dwFlags
	)))
	{
#if _DEBUG
		DebugPrint(__FILE__, "BCryptSetProperty", __LINE__, status);
#endif
		return status;
	}

	// Generate the AES256 key object
	if (!NT_SUCCESS(status = BCryptGenerateSymmetricKey(
		pCryptoInfo->hAlgo,     // _Inout_    BCRYPT_ALG_HANDLE hAlgorithm,
		&(pCryptoInfo->hKey),   // _Out_      BCRYPT_KEY_HANDLE *phKey,
		NULL,                   // _Out_opt_  PUCHAR pbKeyObject,
		0,                      // _In_       ULONG cbKeyObject,
		pCryptoInfo->pbAESKey,  // _In_       PUCHAR pbSecret,
		AES256_KEY_SIZE,        // _In_       ULONG cbSecret,
		0                       // _In_       ULONG dwFlags
	)))
	{
#if _DEBUG
		DebugPrint(__FILE__, "BCryptGenerateSymmetricKey", __LINE__, status);
#endif
		return status;
	}

	return status;
} /* CCryptOpenAESAlgorithmProvider */

CCRYPT_STATUS RSAEncrypt(BCRYPT_KEY_HANDLE hKey, PBYTE pbInput, DWORD dwInput, PBYTE *pbOutput, PDWORD pdwOutput)
{
	CCRYPT_STATUS status = CCRYPT_STATUS_SUCCESS;
	DWORD dwCipherLen = 0;

	// Get length of cipher text
	if (!NT_SUCCESS(status = BCryptEncrypt(
		hKey,       // _Inout_      BCRYPT_KEY_HANDLE hKey,
		pbInput,     // _In_         PUCHAR pbInput,
		dwInput,    // _In_         ULONG cbInput,
		NULL,       // _In_opt_     VOID *pPaddingInfo,
		NULL,       // _Inout_opt_  PUCHAR pbIV,
		0,          // _In_         ULONG cbIV,
		NULL,       // _Out_opt_    PUCHAR pbOutput,
		0,          // _In_         ULONG cbOutput,
		&dwCipherLen,  // _Out_        ULONG *pcbResult,
		BCRYPT_PAD_PKCS1 // _In_         ULONG dwFlags
	)))
	{
#if _DEBUG
		DebugPrint(__FILE__, "BCryptEncrypt", __LINE__, status);
#endif
		return status;
	}

	if (NULL == (*pbOutput = (PBYTE)HeapAlloc(
		GetProcessHeap(),
		HEAP_ZERO_MEMORY,
		dwCipherLen)))
	{
#if _DEBUG
		DebugPrint(__FILE__, "HeapAlloc", __LINE__, CCRYPT_STATUS_HEAP_FAIL);
#endif
		return CCRYPT_STATUS_HEAP_FAIL;
	}

	// Now we can encrypt the text
	if (!NT_SUCCESS(status = BCryptEncrypt(
		hKey,       // _Inout_      BCRYPT_KEY_HANDLE hKey,
		pbInput,     // _In_         PUCHAR pbInput,
		dwInput,    // _In_         ULONG cbInput,
		NULL,       // _In_opt_     VOID *pPaddingInfo,
		NULL,       // _Inout_opt_  PUCHAR pbIV,
		0,          // _In_         ULONG cbIV,
		*pbOutput,   // _Out_opt_    PUCHAR pbOutput,
		dwCipherLen, // _In_         ULONG cbOutput,
		pdwOutput,  // _Out_        ULONG *pcbResult,
		BCRYPT_PAD_PKCS1 // _In_         ULONG dwFlags
	)))
	{
#if _DEBUG
		DebugPrint(__FILE__, "BCryptEncrypt", __LINE__, status);
#endif
		return status;
	}

	// All good!
	return CCRYPT_STATUS_SUCCESS;
} /* RSAEncrypt */

CCRYPT_STATUS RSADecrypt(BCRYPT_KEY_HANDLE hKey, PBYTE pbInput, DWORD dwInput, PBYTE *pbOutput, PDWORD pdwOutput)
{
	DWORD dwResult = 0;
	CCRYPT_STATUS status = CCRYPT_STATUS_SUCCESS;

	if (!NT_SUCCESS(status = BCryptDecrypt(
		hKey,       // _Inout_      BCRYPT_KEY_HANDLE hKey,
		pbInput,     // _In_         PUCHAR pbInput,
		dwInput,    // _In_         ULONG cbInput,
		NULL,       // _In_opt_     VOID *pPaddingInfo,
		NULL,          //_Inout_opt_  PUCHAR pbIV,
		0,          // _In_         ULONG cbIV,
		NULL,       // _Out_opt_    PUCHAR pbOutput,
		0,          // _In_         ULONG cbOutput,
		&dwResult,    // _Out_        ULONG *pcbResult,
		BCRYPT_PAD_PKCS1 // _In_         ULONG dwFlags
	)))
	{
#if _DEBUG
		DebugPrint(__FILE__, "BCryptDecrypt", __LINE__, status);
#endif
		return status;
	}

	if (NULL == (*pbOutput = (PBYTE)HeapAlloc(
		GetProcessHeap(),
		HEAP_ZERO_MEMORY,
		dwResult)))
	{
#if _DEBUG
		DebugPrint(__FILE__, "HeapAlloc", __LINE__, CCRYPT_STATUS_HEAP_FAIL);
#endif
		return CCRYPT_STATUS_HEAP_FAIL;
	}

	if (!NT_SUCCESS(status = BCryptDecrypt(
		hKey,       // _Inout_      BCRYPT_KEY_HANDLE hKey,
		pbInput,     // _In_         PUCHAR pbInput,
		dwInput,    // _In_         ULONG cbInput,
		NULL,       // _In_opt_     VOID *pPaddingInfo,
		NULL,          //_Inout_opt_  PUCHAR pbIV,
		0,          // _In_         ULONG cbIV,
		*pbOutput,   // _Out_opt_    PUCHAR pbOutput,
		dwResult, // _In_         ULONG cbOutput,
		pdwOutput,   // _Out_        ULONG *pcbResult,
		BCRYPT_PAD_PKCS1 // _In_         ULONG dwFlags
	)))
	{
#if _DEBUG
		DebugPrint(__FILE__, "BCryptDecrypt", __LINE__, status);
#endif
		return status;
	}

	return CCRYPT_STATUS_SUCCESS;
} /* RSADecrypt */

CCRYPT_STATUS RSAImportKey(BCRYPT_ALG_HANDLE *phAlgo, BCRYPT_KEY_HANDLE *phKey, LPTSTR lpFileName, LPCWSTR pszBlobType)
{
	//BCRYPT_ALG_HANDLE hAlgo = NULL;
	HANDLE hFile = NULL;
	PBYTE pbBuff = NULL;
	PBYTE pbBase64Decoded = NULL;
	DWORD dwFilesize = 0;
	DWORD dwBase64Decoded = 0;
	CCRYPT_STATUS status = CCRYPT_STATUS_SUCCESS;
	DWORD dwNumBytesRead = 0;

	// Create algorithm provider for RSA
	if (!NT_SUCCESS(status = BCryptOpenAlgorithmProvider(phAlgo, BCRYPT_RSA_ALGORITHM, NULL, 0)))
	{
#if _DEBUG
		DebugPrint(__FILE__, "BCryptOpenAlgorithmProvider", __LINE__, status);
#endif
		goto Cleanup;
	}

	// Create file handle for the key file
	if (INVALID_HANDLE_VALUE == (hFile = CreateFile(
		lpFileName,     // _In_      LPCTSTR lpFileName,
		GENERIC_READ,   // _In_      DWORD dwDesiredAccess,
		0,              // _In_      DWORD dwShareMode,
		NULL,           // _In_opt_  LPSECURITY_ATTRIBUTES lpSecurityAttributes,
		OPEN_EXISTING,  // _In_      DWORD dwCreationDisposition,
		FILE_ATTRIBUTE_NORMAL, // _In_      DWORD dwFlagsAndAttributes,
		NULL // _In_opt_  HANDLE hTemplateFile
	)))
	{
		status = GetLastError();
#if _DEBUG
		DebugPrint(__FILE__, "CreateFile", __LINE__, status);
#endif
		goto Cleanup;
	}

	// Get the file size to determine how much to read in
	if (INVALID_FILE_SIZE == (dwFilesize = GetFileSize(
		hFile, // _In_       HANDLE hFile,
		NULL   // _Out_opt_  LPDWORD lpFileSizeHigh
	)))
	{
		status = GetLastError();
#if _DEBUG
		DebugPrint(__FILE__, "GetFileSize", __LINE__, status);
#endif
		goto Cleanup;
	}

	// Allocate some memory to read in the file
	if (NULL == (pbBuff = (PBYTE)HeapAlloc(
		GetProcessHeap(), // _In_  HANDLE hHeap,
		HEAP_ZERO_MEMORY, // _In_  DWORD dwFlags,
		dwFilesize + 1    // _In_  SIZE_T dwBytes
	)))
	{
		status = CCRYPT_STATUS_HEAP_FAIL;
#if _DEBUG
		DebugPrint(__FILE__, "HeapAlloc", __LINE__, status);
#endif
		goto Cleanup;
	}

	// Now we can read in the file
	if (!ReadFile(
		hFile,				// _In_         HANDLE hFile,
		pbBuff,				// _Out_        LPVOID lpBuffer,
		dwFilesize,			// _In_         DWORD nNumberOfBytesToRead,
		&dwNumBytesRead,	// _Out_opt_    LPDWORD lpNumberOfBytesRead,
		NULL				// _Inout_opt_  LPOVERLAPPED lpOverlapped
	))
	{
		status = GetLastError();
#if _DEBUG
		DebugPrint(__FILE__, "ReadFile", __LINE__, status);
#endif
		goto Cleanup;
	}

	// Base64 decode the contents of the file
	// If successful, we can finally import in the key
	if (Base64Decode((LPSTR)pbBuff, dwFilesize, &pbBase64Decoded, &dwBase64Decoded))
	{
		// Import the key!
		status = BCryptImportKeyPair(
			*phAlgo,     // _In_     BCRYPT_ALG_HANDLE hAlgorithm,
			NULL,        // _Inout_  BCRYPT_KEY_HANDLE hImportKey,
			pszBlobType, // _In_     LPCWSTR pszBlobType,
			phKey,       // _Out_    BCRYPT_KEY_HANDLE *phKey,
			pbBase64Decoded, // _In_     PUCHAR pbInput,
			dwBase64Decoded, // _In_     ULONG cbInput,
			0                // _In_     ULONG dwFlags
		);
	}
	else
	{
		status = CCRYPT_STATUS_BASE64_DECODE_FAIL;
#if _DEBUG
		DebugPrint(__FILE__, "Base64Decode", __LINE__, status);
#endif
	}

Cleanup:

	// Don't need the buffers anymore since we have a handle to the key object now
	if (pbBuff != NULL)
	{
		HeapFree(GetProcessHeap(), 0, pbBuff);
	}
	if (pbBase64Decoded != NULL)
	{
		HeapFree(GetProcessHeap(), 0, pbBase64Decoded);
	}
	if (hFile != NULL)
	{
		CloseHandle(hFile);
	}

	return status;
} /* RSAImportKey */


CCRYPT_STATUS RSAImportKeyFromBuffer(BCRYPT_ALG_HANDLE *phAlgo, BCRYPT_KEY_HANDLE *phKey, PBYTE pbKeyBuffer, DWORD dwKeyBufferSize, LPCWSTR pszBlobType)
{
	PBYTE pbBase64Decoded = NULL;
	DWORD dwBase64Decoded = 0;
	CCRYPT_STATUS status = CCRYPT_STATUS_SUCCESS;

	// Create algorithm provider for RSA
	if (!NT_SUCCESS(status = BCryptOpenAlgorithmProvider(phAlgo, BCRYPT_RSA_ALGORITHM, NULL, 0)))
	{
#if _DEBUG
		DebugPrint(__FILE__, "BCryptOpenAlgorithmProvider", __LINE__, status);
#endif
		goto Cleanup;
	}

	// Base64 decode the contents of the file
	// If successful, we can finally import in the key
	if (Base64Decode((LPSTR)pbKeyBuffer, dwKeyBufferSize, &pbBase64Decoded, &dwBase64Decoded))
	{
		// Import the key!
		status = BCryptImportKeyPair(
			*phAlgo,     // _In_     BCRYPT_ALG_HANDLE hAlgorithm,
			NULL,        // _Inout_  BCRYPT_KEY_HANDLE hImportKey,
			pszBlobType, // _In_     LPCWSTR pszBlobType,
			phKey,       // _Out_    BCRYPT_KEY_HANDLE *phKey,
			pbBase64Decoded, // _In_     PUCHAR pbInput,
			dwBase64Decoded, // _In_     ULONG cbInput,
			0                // _In_     ULONG dwFlags
		);
	}
	else
	{
		status = CCRYPT_STATUS_BASE64_DECODE_FAIL;
#if _DEBUG
		DebugPrint(__FILE__, "Base64Decode", __LINE__, status);
#endif
	}

Cleanup:

	// Don't need the buffers anymore since we have a handle to the key object now
	if (pbBase64Decoded != NULL)
	{
		HeapFree(GetProcessHeap(), 0, pbBase64Decoded);
	}

	return status;
} /* RSAImportKey */


CCRYPT_STATUS RSAExportKey(BCRYPT_KEY_HANDLE hKey, LPCWSTR pszBlobType, LPSTR lpFileName)
{
	HANDLE hFile = NULL;
	PBYTE pbKeyBlob = NULL;
	LPSTR pszBase64Encoded = NULL;
	DWORD dwBase64Encoded;
	DWORD dwResult;
	CCRYPT_STATUS status;

	// Get export blob size
	if (!NT_SUCCESS(status = BCryptExportKey(
		hKey,        // _In_   BCRYPT_KEY_HANDLE hKey,
		NULL,        // _In_   BCRYPT_KEY_HANDLE hExportKey,
		pszBlobType, // _In_   LPCWSTR pszBlobType,
		NULL,        // _Out_  PUCHAR pbOutput,
		0,           // _In_   ULONG cbOutput,
		&dwResult,   // _Out_  ULONG *pcbResult,
		0            // _In_   ULONG dwFlags
	)))
	{
#if _DEBUG
		DebugPrint(__FILE__, "BCryptExportKey", __LINE__, status);
#endif
		goto Cleanup;
	}

	// Allocate memory for the blob
	if (NULL == (pbKeyBlob = (PBYTE)HeapAlloc(
		GetProcessHeap(), // _In_  HANDLE hHeap,
		HEAP_ZERO_MEMORY, // _In_  DWORD dwFlags,
		dwResult // _In_  SIZE_T dwBytes
	)))
	{
		status = CCRYPT_STATUS_HEAP_FAIL;
#if _DEBUG
		DebugPrint(__FILE__, "HeapAlloc", __LINE__, status);
#endif
		goto Cleanup;
	}

	// Now we can export the key
	if (!NT_SUCCESS(status = BCryptExportKey(
		hKey,        // _In_   BCRYPT_KEY_HANDLE hKey,
		NULL,        // _In_   BCRYPT_KEY_HANDLE hExportKey,
		pszBlobType, // _In_   LPCWSTR pszBlobType,
		pbKeyBlob,   // _Out_  PUCHAR pbOutput,
		dwResult,    // _In_   ULONG cbOutput,
		&dwResult,   // _Out_  ULONG *pcbResult,
		0            // _In_   ULONG dwFlags
	)))
	{
#if _DEBUG
		DebugPrint(__FILE__, "BCryptExportKey", __LINE__, status);
#endif
		goto Cleanup;
	}

	// Encode the blob to base64
	if (!Base64Encode(pbKeyBlob, dwResult, &pszBase64Encoded, &dwBase64Encoded)) {
		status = CCRYPT_STATUS_BASE64_DECODE_FAIL;
#if _DEBUG
		DebugPrint(__FILE__, "Base64Encode", __LINE__, status);
#endif
		goto Cleanup;
	}

	if (INVALID_HANDLE_VALUE == (hFile = CreateFileA(
		lpFileName,     // _In_      LPCTSTR lpFileName,
		GENERIC_WRITE,  // _In_      DWORD dwDesiredAccess,
		0,              // _In_      DWORD dwShareMode,
		NULL,           // _In_opt_  LPSECURITY_ATTRIBUTES lpSecurityAttributes,
		OPEN_ALWAYS,    // _In_      DWORD dwCreationDisposition,
		FILE_ATTRIBUTE_NORMAL, // _In_      DWORD dwFlagsAndAttributes,
		NULL            // _In_opt_  HANDLE hTemplateFile
	)))
	{
		status = GetLastError();
#if _DEBUG
		DebugPrint(__FILE__, "CreateFileA", __LINE__, status);
#endif
		goto Cleanup;
	}

	if (FALSE == WriteFile(
		hFile,            // _In_         HANDLE hFile,
		pszBase64Encoded, // _In_         LPCVOID lpBuffer,
		dwBase64Encoded,  // _In_         DWORD nNumberOfBytesToWrite,
		NULL, // _Out_opt_    LPDWORD lpNumberOfBytesWritten,
		NULL  // _Inout_opt_  LPOVERLAPPED lpOverlapped
	))
	{
		status = GetLastError();
#if _DEBUG
		DebugPrint(__FILE__, "WriteFile", __LINE__, status);
#endif
		goto Cleanup;
	}

Cleanup:

	if (pbKeyBlob != NULL)
	{
		HeapFree(GetProcessHeap(), 0, pbKeyBlob);
	}
	if (pszBase64Encoded != NULL)
	{
		HeapFree(GetProcessHeap(), 0, pszBase64Encoded);
	}
	if (hFile != NULL && hFile != INVALID_HANDLE_VALUE)
	{
		CloseHandle(hFile);
	}

	return status;
}

BOOL Base64Decode(LPSTR pszInput, DWORD dwInput, PBYTE *pbOutput, PDWORD pdwOutput)
{
	// Determine size of binary
	if (!CryptStringToBinaryA(
		pszInput,   // _In_     LPCTSTR pszString,
		dwInput,    // _In_     DWORD cchString,
		CRYPT_STRING_BASE64, // _In_     DWORD dwFlags,
		NULL,       // _In_     BYTE *pbBinary,
		pdwOutput,  // _Inout_  DWORD *pcbBinary,
		NULL,       // _Out_    DWORD *pdwSkip,
		NULL        // _Out_    DWORD *pdwFlags
	))
	{
		return FALSE;
	}

	if (NULL == (*pbOutput = (PBYTE)HeapAlloc(
		GetProcessHeap(), // _In_  HANDLE hHeap,
		HEAP_ZERO_MEMORY, // _In_  DWORD dwFlags,
		*pdwOutput        // _In_  SIZE_T dwBytes
	)))
	{
		SetLastError(CCRYPT_STATUS_HEAP_FAIL);
#if _DEBUG
		DebugPrint(__FILE__, "HeapAlloc", __LINE__, CCRYPT_STATUS_HEAP_FAIL);
#endif
		return FALSE;
	}

	// Now we can actually decode stuff
	return CryptStringToBinaryA(
		pszInput,  // _In_     LPCTSTR pszString,
		dwInput,   // _In_     DWORD cchString,
		CRYPT_STRING_BASE64, // _In_     DWORD dwFlags,
		*pbOutput, // _In_     BYTE *pbBinary,
		pdwOutput, // _Inout_  DWORD *pcbBinary,
		NULL,      // _Out_    DWORD *pdwSkip,
		NULL       // _Out_    DWORD *pdwFlags
	);
} /* Base64Decode */

BOOL Base64Encode(PBYTE pbInput, DWORD dwInput, LPSTR *pszOutput, PDWORD pdwOutput)
{
	// Determine size of the base64 string
	if (!CryptBinaryToStringA(
		pbInput,    // _In_       const BYTE *pbBinary,
		dwInput,    // _In_       DWORD cbBinary,
		CRYPT_STRING_BASE64, // _In_       DWORD dwFlags,
		NULL,       // _Out_opt_  LPTSTR pszString,
		pdwOutput   // _Inout_    DWORD *pcchString
	))
	{
#if _DEBUG
		DebugPrint(__FILE__, "CryptBinaryToStringA", __LINE__, 0);
#endif
		return FALSE;
	}

	if (NULL == (*pszOutput = (LPSTR)HeapAlloc(
		GetProcessHeap(), // _In_  HANDLE hHeap,
		HEAP_ZERO_MEMORY, // _In_  DWORD dwFlags,
		*pdwOutput // _In_  SIZE_T dwBytes
	)))
	{
		SetLastError(CCRYPT_STATUS_HEAP_FAIL);
#if _DEBUG
		DebugPrint(__FILE__, "HeapAlloc", __LINE__, CCRYPT_STATUS_HEAP_FAIL);
#endif
		return FALSE;
	}

	return CryptBinaryToStringA(
		pbInput,    // _In_       const BYTE *pbBinary,
		dwInput,    // _In_       DWORD cbBinary,
		CRYPT_STRING_BASE64, // _In_       DWORD dwFlags,
		*pszOutput, // _Out_opt_  LPTSTR pszString,
		pdwOutput   // _Inout_    DWORD *pcchString
	);
} /* Base64Encode */

CCRYPT_STATUS CCryptHmac(PBYTE pbInput, DWORD dwInput, PBYTE *pbOutput, PCHAR pSecret, DWORD dwSecret)
{
	BCRYPT_ALG_HANDLE hAlgo = NULL;
	BCRYPT_HASH_HANDLE hHash = NULL;
	CCRYPT_STATUS status = CCRYPT_STATUS_SUCCESS;
	DWORD dwHash = 0;
	DWORD dwResult = 0;

	// Initiate a algorithm provider for SHA256 HMAC'ing
	if (!NT_SUCCESS(status = BCryptOpenAlgorithmProvider(
		&hAlgo,
		BCRYPT_SHA256_ALGORITHM,
		NULL,
		BCRYPT_ALG_HANDLE_HMAC_FLAG
	)))
	{
#if _DEBUG
		DebugPrint(__FILE__, "BCryptOpenAlgorithmProvider", __LINE__, status);
#endif
		goto Cleanup;
	}

	// Get handle to the hash object
	if (!NT_SUCCESS(status = BCryptCreateHash(hAlgo, &hHash, NULL, 0, (PUCHAR)pSecret, dwSecret, 0)))
	{
#if _DEBUG
		DebugPrint(__FILE__, "BCryptCreateHash", __LINE__, status);
#endif
		goto Cleanup;
	}

	// Hash the input
	if (!NT_SUCCESS(status = BCryptHashData(hHash, pbInput, dwInput, 0)))
	{
#if _DEBUG
		DebugPrint(__FILE__, "BCryptHashData", __LINE__, status);
#endif
		goto Cleanup;
	}

	// Get size of hash that we are using in bytes
	// We use this size to allocate the correct amount of memory
	// for our hash buffer
	if (!NT_SUCCESS(status = BCryptGetProperty(
		hAlgo,
		BCRYPT_HASH_LENGTH,
		(PUCHAR)&dwHash,
		sizeof(DWORD),
		&dwResult,
		0
	)))
	{
#if _DEBUG
		DebugPrint(__FILE__, "BCryptGetProperty", __LINE__, status);
#endif
		goto Cleanup;
	}

	// Allocate hash buffer
	if (NULL == (*pbOutput = (PBYTE)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, dwHash)))
	{
		status = CCRYPT_STATUS_HEAP_FAIL;
#if _DEBUG
		DebugPrint(__FILE__, "HeapAlloc", __LINE__, status);
#endif
		goto Cleanup;
	}

	// Now we can actually get the hash value
	if (!NT_SUCCESS(status = BCryptFinishHash(hHash, *pbOutput, dwHash, 0)))
	{
#if _DEBUG
		DebugPrint(__FILE__, "BCryptFinishHash", __LINE__, status);
#endif
	}

Cleanup:

	if (hHash != NULL)
	{
		BCryptDestroyHash(hHash);
	}
	if (hAlgo != NULL)
	{
		BCryptCloseAlgorithmProvider(hAlgo, 0);
	}

	return status;
} /* CreateHmac */

BOOL SubString(PBYTE pBuffer, DWORD dwStart, DWORD dwLen, PBYTE pResults)
{
	CopyMemory(
		pResults, // _In_  PVOID Destination,
		(pBuffer + dwStart), // _In_  const VOID *Source,
		dwLen     // _In_  SIZE_T Length
	);

	return TRUE;
} /* SubString */

BOOL TestMagicValue(PBYTE pbMagic)
{
	if (pbMagic[0] != MAGIC_CHAR_1 &&
		pbMagic[1] != MAGIC_CHAR_2 &&
		pbMagic[2] != MAGIC_CHAR_2 &&
		pbMagic[3] != MAGIC_CHAR_1
		)
	{
		return FALSE;
	}

	return TRUE;
} /* TestMagicValue */

BOOL ByteCompare(PBYTE pbBuffer1, DWORD dwCount1, PBYTE pbBuffer2, DWORD dwCount2)
{
	// Make sure the count for each buffers are the same
	if (dwCount1 != dwCount2)
	{
		return FALSE;
	}

	for (DWORD i = 0; i < dwCount1; i++)
	{
		if ((UCHAR)pbBuffer1[i] != (UCHAR)pbBuffer2[i])
		{
			return FALSE;
		}
	}

	return TRUE;
}

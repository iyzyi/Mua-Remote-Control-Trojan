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

#ifndef _DSH_LIBCRYPTO_H_
#define _DSH_LIBCRYPTO_H_

#pragma comment(lib, "Crypt32.lib")
#pragma comment(lib, "Bcrypt.lib")

#define NT_SUCCESS(status) (((NTSTATUS)(status)) >= 0)

typedef NTSTATUS CCRYPT_STATUS;
#define CCRYPT_STATUS_SUCCESS 0x0
#define CCRYPT_STATUS_HEAP_FAIL 0xf0000001
#define CCRYPT_STATUS_HEAP_REALLOC_FAIL 0xf0000002
#define CCRYPT_STATUS_INVALID_HANDLE 0xf0000003
#define CCRYPT_STATUS_FILE_FINALIZED 0xf0000004
#define CCRYPT_STATUS_BASE64_DECODE_FAIL 0xf0000005
#define CCRYPT_STATUS_BASE64_ENCODE_FAIL 0xf0000006
#define CCRYPT_STATUS_INVALID_FILE 0xf0000007
#define CCRYPT_STATUS_INVALID_HMAC 0xf0000008
#define CCRYPT_STATUS_ALGO_FAIL 0xf0000009
#define CCRYPT_STATUS_USE_DEFAULT 0xf0009999

#define AES256_KEY_SIZE 32
#define AES256_ENCRYPTED_KEY_SIZE 256
#define HMAC_SIZE 32
#define MAGIC_SIZE 4
#define MAGIC_CHAR_1 '\x7f'
#define MAGIC_CHAR_2 '\x07'
#define CCRYPT_FILE_READ 0
#define CCRYPT_FILE_WRITE 1

typedef struct _FILE_CRYPTO_INFO {
	HANDLE hFile;               // Handle to the file to read/write and encrypt/decrypt
	BCRYPT_ALG_HANDLE hAlgo;    // Handle to AES algorithm provider
	BCRYPT_KEY_HANDLE hKey;     // Handle to AES crypto key
	PBYTE pbAESKey;             // String value of AES key
} FILE_CRYPTO_INFO, *PFILE_CRYPTO_INFO;

/**
 * Create or open a file for cryptologic transactions. This will initialize the
 * FILE_CRYPTO_INFO structure that will be used for the file's operational lifecycle.
 *
 * @param   Out     PFILE_CRYPTO_INFO   Pointer to a FILE_CRYPTO_INFO structure
 * @param   In      LPCTSTR             The name of the file to be created or opened
 * @param   In      DWORD               Create mode. This will be either CCRYPT_FILE_READ or CCRYPT_FILE_WRITE
 * @return
 */
CCRYPT_STATUS CCryptCreateFile(PFILE_CRYPTO_INFO pCryptoInfo, LPCTSTR lpFileName, DWORD dwMode);

/**
 * Read a file and decrypt it's content.
 *
 * @param   In      PFILE_CRYPTO_INFO   Pointer to a FILE_CRYPTO_INFO structure that was initialized
 *                                      by calling CCryptCreateFile()
 * @param   In      BCRYPT_KEY_HANDLE   Handle to a RSA crypto key imported from RSAImportKey()
 * @param   Out     LPBYTE              Pointer to a buffer to hold the contents of the decrypted file
 * @param   Out_opt PDWORD              Returns the number of bytes read
 * @return
 */
BOOL CCryptReadFile(PFILE_CRYPTO_INFO pCryptoInfo, BCRYPT_KEY_HANDLE hKey, LPBYTE *pbBuffer, PDWORD pdwBytesRead);

/**
 * Write contents to the file to encrypt
 *
 * @param   In      PFILE_CRYPTO_INFO   Pointer to a FILE_CRYPTO_INFO structure that was initialized
 *                                      by calling CCryptCreateFile()
 * @param   In      LPBYTE              Pointer to a buffer to encrypt
 * @param   In      DWORD               Size of pbBuffer in bytes
 * @param   Out_opt PDWORD              Returns the number of bytes written
 * @return
 */
BOOL CCryptWriteFile(PFILE_CRYPTO_INFO pCryptoInfo, LPBYTE pbBuffer, DWORD dwBytesToWrite, PDWORD pdwBytesWritten);

/**
 * This function is used to finalize the write to the encrypted file. It must be called
 * after all desired CCryptWriteFile() have been completed
 *
 * @param   In      PFILE_CRYPTO_INFO   Pointer to a FILE_CRYPTO_INFO structure that was initialized
 *                                      by calling CCryptCreateFile()
 * @param   In      BCRYPT_KEY_HANDLE   Handle to a RSA crypto key imported from RSAImportKey()
 * @return
 */
BOOL CCryptFinalizeFile(PFILE_CRYPTO_INFO pCryptoInfo, BCRYPT_KEY_HANDLE hKey);

/**
 * Closes the file handle and cleans up associated resources
 *
 * @param   In      PFILE_CRYPTO_INFO   Pointer to a FILE_CRYPTO_INFO structure that was initialized
 *                                      by calling CCryptCreateFile()
 * @return
 */
VOID CCryptCloseHandle(PFILE_CRYPTO_INFO pCryptoInfo);

/**
 * Initializes an AES crypto key. A string key must be created at PFILE_CRYPTO_INFO->pbAESKey
 * prior to calling this function.
 *
 * This is an internal function and should generally not be called on it's own
 *
 * @param   In      PFILE_CRYPTO_INFO   Pointer to a FILE_CRYPTO_INFO structure
 * @return
 */
CCRYPT_STATUS CCryptOpenAESAlgorithmProvider(PFILE_CRYPTO_INFO pCryptoInfo);

/**
 * Encrypt the given buffer with RSA
 *
 * @param   In      BCRYPT_KEY_HANDLE   Handle to a RSA crypto key imported from RSAImportKey()
 * @param   In      PBYTE               Pointer to a buffer to encrypt
 * @param   In      DWORD               Size of pInput in bytes
 * @param   Out     PBYTE               Pointer to a buffer that will recieve the encypted text
 * @param   Out_opt PDWORD              Size of pbOutput in bytes
 * @return
 */
CCRYPT_STATUS RSAEncrypt(BCRYPT_KEY_HANDLE hKey, PBYTE pInput, DWORD dwInput, PBYTE *pbOutput, PDWORD pdwOutput);

/**
 * Decrypt the given buffer with RSA
 *
 * @param   In      BCRYPT_KEY_HANDLE   Handle to a RSA crypto key imported from RSAImportKey()
 * @param   In      PBYTE               Pointer to a buffer with the encrypted data
 * @param   In      DWORD               Size of pInput in bytes
 * @param   Out     PBYTE               Pointer to a buffer that will recieve the decrypted text
 * @param   Out_opt PDWORD              Size of pbOutput in bytes
 * @return
 */
CCRYPT_STATUS RSADecrypt(BCRYPT_KEY_HANDLE hKey, PUCHAR pInput, DWORD dwInput, PBYTE *pOutput, PDWORD pdwOutput);

/**
 * Import an RSA key pair
 *
 * @param   Out     BCRYPT_ALG_HANDLE   Pointer to a BCRYPT_ALG_HANDLE handle
 * @param   Out     BCRYPT_KEY_HANDLE   Pointer to a BCRYPT_KEY_HANDLE handle
 * @param   In      LPTSTR              Name of the key file to import
 * @param   In      LPCWSTR             Unicode string that contains an identifier that specifies the
 *                                      type of BLOB that is contained in the pbInput buffer.
 *                                      This can only be either BCRYPT_RSAPRIVATE_BLOB or BCRYPT_RSAPUBLIC_BLOB
 * @return
 */
CCRYPT_STATUS RSAImportKey(BCRYPT_ALG_HANDLE *phAlgo, BCRYPT_KEY_HANDLE *phKey, LPTSTR lpFileName, LPCWSTR pszBlobType);

// 自己改动的函数
CCRYPT_STATUS RSAImportKeyFromBuffer(BCRYPT_ALG_HANDLE *phAlgo, BCRYPT_KEY_HANDLE *phKey, PBYTE pbKeyBuffer, DWORD dwKeyBufferSize, LPCWSTR pszBlobType);

/**
 * Export an RSA key pair
 *
 * @param   In      BCRYPT_KEY_HANDLE   Pointer to a BCRYPT_KEY_HANDLE handle
 * @param   In      LPCWSTR             Unicode string that contains an identifier that specifies the
 *                                      type of BLOB that is contained in the pbInput buffer.
 *                                      This can only be either BCRYPT_RSAPRIVATE_BLOB or BCRYPT_RSAPUBLIC_BLOB
 * @param   In      LPSTR               Name of the file to save the export key
 * @return
 */
CCRYPT_STATUS RSAExportKey(BCRYPT_KEY_HANDLE hKey, LPCWSTR pszBlobType, LPSTR lpFileName);

/**
 * Decode a base64 encoded string
 *
 * @param   In      LPSTR       Pointer to an base64 encoded string
 * @param   In      DWORD       Size of pszInput in bytes
 * @param   Out     PBYTE       Pointer to a buffer that will recieve the decoded string
 * @param   Out     PDWORD      Size of pbOutput in bytes
 * @return
 */
BOOL Base64Decode(LPSTR pszInput, DWORD dwInput, PBYTE *pbOutput, PDWORD pdwOutput);

/**
 * Encode a buffer to base64
 *
 * @param   In      PBYTE       Pointer to a buffer to base64 encode
 * @param   In      DWORD       Size of pbInput in bytes
 * @param   Out     LPSTR       Pointer to a buffer that will recieve the encoded string
 * @param   Out     PDWORD      Size of pszOutput in bytes
 * @return
 */
BOOL Base64Encode(PBYTE pbInput, DWORD dwInput, LPSTR *pszOutput, PDWORD pdwOutput);

/**
 * Create a Hash-based Message Authentication Code (HMAC) for the given buffer
 *
 * @param   In      PBYTE       Pointer to the buffer to HMAC
 * @param   In      DWORD       Size of pbInput in bytes
 * @param   Out     PBYTE       Pointer to a buffer that will recieve the HMAC
 * @param   In      PCHAR       A secret key to use to generate the HMAC
 * @param   In      DWORD       Size of pSecret in bytes
 * @return
 */
CCRYPT_STATUS CCryptHmac(PBYTE pbInput, DWORD dwInput, PBYTE *pbOutput, PCHAR pSecret, DWORD dwSecret);

/**
 * @param   In
 * @param   In
 * @param   In
 * @param   Out
 * @return
 */
BOOL SubString(PBYTE pBuffer, DWORD dwStart, DWORD dwLen, PBYTE pResults);

/**
 * This is a helper function to check for our "magic" value
 *
 * @param   In      PBYTE   Buffer to check
 * @return
 */
BOOL TestMagicValue(PBYTE pbMagic);

/**
 * Perform a simple byte comparison of two buffers
 *
 * @param   In      PBYTE   Pointer to buffer 1
 * @param   In      DWORD   Size of pbBuffer1 in bytes
 * @param   In      PBYTE   Pointer to buffer 2
 * @param   In      DWORD   Size of pbBuffer2 in bytes
 * @return
 */
BOOL ByteCompare(PBYTE pbBuffer1, DWORD dwCount1, PBYTE pbBuffer2, DWORD dwCount2);

#endif /* _DSH_LIBCRYPTO_H_ */

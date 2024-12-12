#pragma once

#include "AES.h"


enum CRYPTO_ALGORITHM_ID {
	PLAINTEXT,
	AES_128_CFB,

	// TODO: 其他可选的加密算法，如ChaCha20-Poly1305和AES-128-GCM
};


class CCrypto {
public:
	CRYPTO_ALGORITHM_ID	m_dwCryptoAlgorithmId;

	BOOL m_bInitSuccess;

	AES  m_AesEncrypt;
	AES  m_AesDecrypt;
	
	// TODO: 假设再引入其他加密算法，则如下
	// ChaCha	m_ChaChaEncrypt
	// ChaCha	m_ChaChaDecrypt
	

public:
	CCrypto(CRYPTO_ALGORITHM_ID dwCryptoAlgorithmId, PBYTE pbRsaEncrypted);
	CCrypto();
	~CCrypto();

	DWORD Encrypt(PBYTE pbPlaintext, DWORD dwPlaintextLength, PBYTE pbCiphertext);
	DWORD Decrypt(PBYTE pbCiphertext, DWORD dwCiphertextLength, PBYTE pbPlaintext);

	// 不同加密算法的填充长度不同。该函数输入明文长度，返回密文长度
	DWORD GetCiphertextLength(DWORD dwPlaintextLength);

	// RSA解密出通信密钥
	void RsaDecryptKey(PBYTE pbCiphertext, DWORD dwKeyLength, PBYTE pbDecryptedKey, PDWORD pdwDecryptedKeyLength);
};
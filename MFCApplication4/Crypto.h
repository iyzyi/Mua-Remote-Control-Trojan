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

	AES  m_AesEncrypt;
	AES  m_AesDecrypt;
	
	// TODO: 假设再引入其他加密算法，则如下
	// ChaCha	m_ChaChaEncrypt
	// ChaCha	m_ChaChaDecrypt
	

public:
	CCrypto(CRYPTO_ALGORITHM_ID dwCryptoAlgorithmId, PBYTE pbKey=NULL, PBYTE pbIv=NULL);
	CCrypto();
	~CCrypto();

	PBYTE Encrypt(PBYTE pbData, DWORD dwInLength, DWORD *pdwOutLength);
	PBYTE Decrypt(PBYTE pbData, DWORD dwInLength, DWORD *pdwOutLength);
};
#pragma once

#include "AES.h"

class CCrypto {
	CCrypto(CRYPTO_ALGORITHM_ID dwCryptoAlgorithmId);
	~CCrypto();

	PBYTE CCrypto::Encrypt(PBYTE pbData, DWORD dwInLength, DWORD &pdwOutLength);
	PBYTE CCrypto::Decrypt(PBYTE pbData, DWORD dwInLength, DWORD &pdwOutLength);
};


enum CRYPTO_ALGORITHM_ID {
	CRYPTO_ID_AES,
	// TODO: 其他可选的加密算法
};
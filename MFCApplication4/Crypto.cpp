#include "pch.h"
#include "Crypto.h"
#include "Misc.h"


CCrypto::CCrypto(CRYPTO_ALGORITHM_ID dwCryptoAlgorithmId, PBYTE pbKey, PBYTE pbIv) {
	m_dwCryptoAlgorithmId = dwCryptoAlgorithmId;
	switch (m_dwCryptoAlgorithmId) {

	case PLAINTEXT:
		break;

	case AES_128_CFB:
		m_AesEncrypt = AES(128, pbKey, pbIv);
		m_AesDecrypt = AES(128, pbKey, pbIv);
		break;
		
	// TODO: 其他加密算法，如ChaCha20-Poly1305
	}
}


CCrypto::~CCrypto() {

}


PBYTE CCrypto::Encrypt(PBYTE pbData, DWORD dwInLength, DWORD *pdwOutLength) {
	switch (m_dwCryptoAlgorithmId) {
	
	case PLAINTEXT:
		*pdwOutLength = dwInLength;
		return CopyBuffer(pbData, dwInLength);

	case AES_128_CFB:
		return m_AesEncrypt.EncryptCFB(pbData, dwInLength, pdwOutLength);
		break;

	default:
		*pdwOutLength = 0;
		return NULL;
	}


}


PBYTE CCrypto::Decrypt(PBYTE pbData, DWORD dwInLength, DWORD *pdwOutLength) {
	return NULL;
}
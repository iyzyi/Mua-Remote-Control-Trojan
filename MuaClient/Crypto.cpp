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


CCrypto::CCrypto() {

}


CCrypto::~CCrypto() {

}


// 返回密文字节数。加密后的密文写入第三个参数指向的缓冲区。
DWORD CCrypto::Encrypt(PBYTE pbPlaintext, DWORD dwPlaintextLength, PBYTE pbCiphertext) {
	DWORD dwCiphertextLength = 0;

	switch (m_dwCryptoAlgorithmId) {

	case PLAINTEXT:
		memcpy(pbCiphertext, pbPlaintext, dwPlaintextLength);
		dwCiphertextLength = dwPlaintextLength;
		return dwCiphertextLength;

	case AES_128_CFB:
		dwCiphertextLength = m_AesEncrypt.EncryptCFB(pbPlaintext, dwPlaintextLength, pbCiphertext);
		return dwCiphertextLength;

	default:
		return 0;
	}
}


// 返回明文字节数。解密后的明文写入第三个参数指向的缓冲区
DWORD CCrypto::Decrypt(PBYTE pbCiphertext, DWORD dwCiphertextLength, PBYTE pbPlaintext) {
	DWORD dwPlaintextLength = 0;

	switch (m_dwCryptoAlgorithmId) {

	case PLAINTEXT:
		memcpy(pbPlaintext, pbCiphertext, dwCiphertextLength);
		dwPlaintextLength = dwCiphertextLength;
		return dwPlaintextLength;

	case AES_128_CFB:
		dwPlaintextLength = m_AesDecrypt.DecryptCFB(pbCiphertext, dwCiphertextLength, pbPlaintext);
		return dwPlaintextLength;

	default:
		return 0;
	}
}


// 不同加密算法的填充长度不同。该函数输入明文长度，返回密文长度
DWORD CCrypto::GetCiphertextLength(DWORD dwPlaintextLength) {
	switch (m_dwCryptoAlgorithmId) {

	case PLAINTEXT:
		return dwPlaintextLength;

	case AES_128_CFB:
		return m_AesEncrypt.GetPaddingLength(dwPlaintextLength);

	default:
		return 0;
	}
}
#include "pch.h"
#include "Crypto.h"
#include "Misc.h"
#include "RSA.h"


CCrypto::CCrypto(CRYPTO_ALGORITHM_ID dwCryptoAlgorithmId, PBYTE pbKey, PBYTE pbIv) {
	m_dwCryptoAlgorithmId = dwCryptoAlgorithmId;

	switch (m_dwCryptoAlgorithmId) {

	case PLAINTEXT:
		break;

	case AES_128_CFB:
		m_AesEncrypt = AES(128, pbKey, pbIv);
		m_AesDecrypt = AES(128, pbKey, pbIv);

		BYTE pbKeyAndIv[32];
		memcpy(pbKeyAndIv, pbKey, 16);
		memcpy(pbKeyAndIv + 16, pbIv, 16);

		DWORD dwOutLength = 0;
		RsaEncryptKey(pbKeyAndIv, 32, m_pbRsaEncrypted, &dwOutLength);
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



// 使用RSA加密 对称密码/序列密码的密钥
int CCrypto::RsaEncryptKey(PBYTE pbKey, DWORD dwKeyLength, PBYTE pbCiphertext, PDWORD pdwCiphertextLength) {
	// 2048bit的RSA，可加密256Byte，PKCS1填充要占据11Byte
	assert(dwKeyLength <= 256 - 11);

	// 这个密钥可以通过RSA.cpp中的RsaKeyGen()生成
	BYTE pbPublicKey[1000] = "UlNBMQAIAAADAAAAAAEAAAAAAAAAAAAAAQAB0JnyWyZ4HbLMub0vP1Pvqo0/DiYY"\
		"G16Zv525VQiw39F3YIbcS8v635lxR2qcW9eafrsphmI3wq7SNLxvKXTsVdzCoLCD"\
		"McAtBjySPsmoOGVL2u+sHcX1Nan3Qn9wyOrAX8oR9hIlml4xszJcKQMpGHkuvy6E"\
		"X3oR0jmsv6S36CprS4MjOvU1fEnMrZaVJsd8jI09DYZjp4cQZHCKkj8uPJdmwoqJ"\
		"o72kIDpKXaP89zfnMMjv61rslXQu0ncGH/Yo4nhLxQLL98SzG29i31tNFpZb7xUj"\
		"3ibSa6zlMNkIRz24MM08wWSbQkBspEpWys3FZ3bVTqxcV2SxkwhVj8kYbQ==";

	RsaEncrypt(pbKey, dwKeyLength, pbCiphertext, pdwCiphertextLength, pbPublicKey, strlen((char*)pbPublicKey));
	return 0;
}
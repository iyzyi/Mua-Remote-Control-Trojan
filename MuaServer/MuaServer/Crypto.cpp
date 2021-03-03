#include "pch.h"
#include "Crypto.h"
#include "Misc.h"
#include "RSA.h"


CCrypto::CCrypto(CRYPTO_ALGORITHM_ID dwCryptoAlgorithmId, PBYTE pbRsaEncrypted) {
	m_bInitSuccess = true;

	m_dwCryptoAlgorithmId = dwCryptoAlgorithmId;

	BYTE  pbRsaDecrypted[256];
	DWORD dwKeyLength = 0;
	RsaDecryptKey(pbRsaEncrypted, 256, pbRsaDecrypted, &dwKeyLength);

	switch (m_dwCryptoAlgorithmId) {

	case PLAINTEXT:
		break;

	case AES_128_CFB:
		if (dwKeyLength != 32) {
			m_bInitSuccess = false;
		}
		else {
			BYTE pbKey[16];
			BYTE pbIv[16];
			memcpy(pbKey, pbRsaDecrypted, 16);
			memcpy(pbIv, pbRsaDecrypted + 16, 16);

			m_AesEncrypt = AES(128, pbKey, pbIv);
			m_AesDecrypt = AES(128, pbKey, pbIv);
		}
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



void CCrypto::RsaDecryptKey(PBYTE pbCiphertext, DWORD dwCiphertextLength, PBYTE pbDecryptedKey, PDWORD pdwDecryptedKeyLength) {
	// 这个密钥可以通过RSA.cpp中的RsaKeyGen()生成
	BYTE pbPrivateKey[1000] = "UlNBMgAIAAADAAAAAAEAAIAAAACAAAAAAQABvm1ftWn4my/SWAfH9MUG8pAeWCYs"\
		"ooqG62Mb8NrbiC8iF4E8acmFcFizl35zTpryO18PSSu1PnUrj+4QeWe2kykcudcX"\
		"Qk13jy43q8VgIiJlzdSWlN81EftyLA/fFw2oT2+qf40wfNf0/VnYi64kLycX3x0v"\
		"GKnSxhSvdaZm0osv62E+er7stKD6pf37LAP4STAxIEgRfrN52aJk0wuFT8//o6Wx"\
		"kQktygCdcCCn278+sHvdTiefgrMwjL1i2saQNSjOAKTUrInevcR25+piofE8Jq2P"\
		"RzjeanQxm9urekrdAX1iVie1a223kZyuVz6ys3KNKnibkuzsbSqAhzLm2c0oE+GP"\
		"ZMgZIdOZswz3Ad3N0BaqNKCCRIN112jwbP1fYIG2QoeH0GUbgz9DEBU10MGXgR+M"\
		"r4Z32RFhI9PtxAxjfOe/lQY6qkhUJWjUghG0h4p+rKlaIcYSRYsjgU1pMJfV7sjL"\
		"eWQ8r6qIiW2xtct66mfcfxYFYuc4bb95VB2X7Z7P2QTwwQ1GlNwxirz8mDBjX1sN"\
		"7A3lgHKtvw1tUaE480+hP0uaU17Da8HflGwpZUt5k3/SvfyPnImuwb6jrYCdZTk0"\
		"Cvmp5pxzm6mMv8fOSnfIG+Z1NMQ7sWgI5PP2BobnTYGf7veKac2F8RhrY28wwNHY"\
		"61bMVS/tayHvjQ8=";

	RsaDecrypt(pbCiphertext, dwCiphertextLength, pbDecryptedKey, pdwDecryptedKeyLength, pbPrivateKey, strlen((char*)pbPrivateKey));
}
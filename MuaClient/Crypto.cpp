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

	BYTE pbPublicKey[1000] = "UlNBMQAIAAADAAAAAAEAAAAAAAAAAAAAAQABvm1ftWn4my/SWAfH9MUG8pAeWCYs"\
		"ooqG62Mb8NrbiC8iF4E8acmFcFizl35zTpryO18PSSu1PnUrj+4QeWe2kykcudcX"\
		"Qk13jy43q8VgIiJlzdSWlN81EftyLA/fFw2oT2+qf40wfNf0/VnYi64kLycX3x0v"\
		"GKnSxhSvdaZm0osv62E+er7stKD6pf37LAP4STAxIEgRfrN52aJk0wuFT8//o6Wx"\
		"kQktygCdcCCn278+sHvdTiefgrMwjL1i2saQNSjOAKTUrInevcR25+piofE8Jq2P"\
		"RzjeanQxm9urekrdAX1iVie1a223kZyuVz6ys3KNKnibkuzsbSqAhzLm2Q==";

	RsaEncrypt(pbKey, dwKeyLength, pbCiphertext, pdwCiphertextLength, pbPublicKey, strlen((char*)pbPublicKey));
	return 0;
}
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
	BYTE pbPrivateKey[1000] = "UlNBMgAIAAADAAAAAAEAAIAAAACAAAAAAQAB0JnyWyZ4HbLMub0vP1Pvqo0/DiYY"\
		"G16Zv525VQiw39F3YIbcS8v635lxR2qcW9eafrsphmI3wq7SNLxvKXTsVdzCoLCD"\
		"McAtBjySPsmoOGVL2u+sHcX1Nan3Qn9wyOrAX8oR9hIlml4xszJcKQMpGHkuvy6E"\
		"X3oR0jmsv6S36CprS4MjOvU1fEnMrZaVJsd8jI09DYZjp4cQZHCKkj8uPJdmwoqJ"\
		"o72kIDpKXaP89zfnMMjv61rslXQu0ncGH/Yo4nhLxQLL98SzG29i31tNFpZb7xUj"\
		"3ibSa6zlMNkIRz24MM08wWSbQkBspEpWys3FZ3bVTqxcV2SxkwhVj8kYbfaxisIG"\
		"HtlAqcoXw4O6x0l5yRpHy17YVPCVv0L+rsy767ayHOAqvojSJc7VaNaVMadNGEY+"\
		"lPe8i1iUa9MB8RwcWNeU4t1o1ZdQIAOaq37IiY7RLzOfqTgP0O/htwPpWxL7hAF9"\
		"JP5xI/OlaxP3wO2HdzMrRJwrKPjDlqBhrSav2HiG9m0VgmC9GreyHJWVEYEl5znx"\
		"Um1b1eYWDqHEBIg8HoWdh/5bVnn1NK9XIcdhOSkD/Ejqtya5Nz7J8qccjoKVPhGM"\
		"9D/QrWRKUv1r5fUo/F+eID6sE9QgCbkonE3/dvmSKkIgKWNQ5sz0FSHLjFi3HCIz"\
		"ims3YZOK/LD7uaM=";

	RsaDecrypt(pbCiphertext, dwCiphertextLength, pbDecryptedKey, pdwDecryptedKeyLength, pbPrivateKey, strlen((char*)pbPrivateKey));
}
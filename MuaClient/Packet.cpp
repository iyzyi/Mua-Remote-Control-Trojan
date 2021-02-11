#include "pch.h"
#include "Packet.h"
#include "SocketClient.h"


// 统一声明：
// 本类中的封包包括封包包头和封包包体
// 封包包头的定义详见PACKET_HEAD结构体
// 并不包括开头的4字节长度哦，这个是由HP-Socket负责的




CPacket::CPacket() {

	m_dwPacketLength = 0;				// 整个封包的长度(包括包头和包体，但不包括封包中表示长度的4个字节)
	m_PacketHead = PACKET_HEAD();	// 包头
	m_pbPacketBody = NULL;				// 包体

	m_dwPacketBodyLength = 0;				// 包体长度

	m_pbPacketPlaintext = NULL;				// 封包明文数据（照例不包括开头表示长度的4字节）
	m_pbPacketCiphertext = NULL;				// 封包密文数据（照例不包括开头表示长度的4字节）
}


CPacket::CPacket(CSocketClient* pSocketClient) {
	m_dwConnId = 0;
	m_pSocketClient = pSocketClient;
	m_pCrypto = &(m_pSocketClient->m_Crypto);

	m_dwPacketLength = 0;
	m_PacketHead = PACKET_HEAD();
	m_pbPacketBody = NULL;

	m_dwPacketBodyLength = 0;

	m_pbPacketPlaintext = NULL;
	m_pbPacketCiphertext = NULL;
}


CPacket::~CPacket() {
	if (m_pbPacketBody) {		// 如果这里不是用xmalloc，而是直接用栈，那这里free必然崩溃。还在想怎么改。暂时先不允许栈吧，PacketBody都得xmalloc申请。// 现在改成new和delete
		delete m_pbPacketBody;
	}

	if (m_pbPacketPlaintext) {
		delete m_pbPacketPlaintext;
	}

	if (m_pbPacketCiphertext) {
		delete m_pbPacketCiphertext;
	}
}


// 解析封包，不可与PacketCombine在同一个Packet对象中同时使用
VOID CPacket::PacketParse(PBYTE pbData, DWORD dwPacketLength) {
	m_dwPacketLength = dwPacketLength;
	m_dwPacketBodyLength = m_dwPacketLength - PACKET_HEAD_LENGTH;

	// 复制封包（密文）
	m_pbPacketCiphertext = new BYTE[dwPacketLength];
	memcpy(m_pbPacketCiphertext, pbData, dwPacketLength);

	// 解密封包。接收到的封包是密文，解密出来的明文长度一定比密文短，
	// 所以这里直接用密文的长度dwPacketLength就足够了
	m_pbPacketPlaintext = new BYTE[dwPacketLength];
	DWORD dwPacketCiphertextLength = dwPacketLength;
	DWORD dwPacketPlaintextLength = m_pCrypto->Decrypt(m_pbPacketCiphertext, dwPacketCiphertextLength, m_pbPacketPlaintext);

	// 封包长度更新为解密后的明文封包的长度
	m_dwPacketLength = dwPacketPlaintextLength;
	m_dwPacketBodyLength = m_dwPacketLength - PACKET_HEAD_LENGTH;

	// 解析包头
	m_PacketHead = PACKET_HEAD((PBYTE)m_pbPacketPlaintext);

	// 拷贝包体
	m_pbPacketBody = new BYTE[m_dwPacketBodyLength];
	memcpy(m_pbPacketBody, m_pbPacketPlaintext + PACKET_HEAD_LENGTH, m_dwPacketBodyLength);
}


// 组装封包，不可与PacketParse在同一个Packet对象中同时使用
VOID CPacket::PacketCombine(COMMAND_ID wCommandId, PBYTE pbPacketBody, DWORD dwPacketBodyLength) {
	m_dwPacketLength = PACKET_HEAD_LENGTH + dwPacketBodyLength;
	m_dwPacketBodyLength = dwPacketBodyLength;

	// 设置（计算）包头
	m_PacketHead.wCommandId = wCommandId;
	m_PacketHead.dwCheckSum = 0;				// 暂不引入校验
	m_PacketHead.bySplitNum = 0;				// 暂不考虑分片

	// 包头转成buffer形式
	BYTE pbPacketHead[PACKET_HEAD_LENGTH];
	m_PacketHead.StructToBuffer(pbPacketHead);

	if (m_dwPacketBodyLength > 0) {
		// 拷贝包体
		m_pbPacketBody = new BYTE[dwPacketBodyLength];
		memcpy(m_pbPacketBody, pbPacketBody, dwPacketBodyLength);
	}

	// 组包，不过不包括前4字节，因为HP-Socket的Pack模式，在收发数据的时候会自动添上或删去
	m_pbPacketPlaintext = new BYTE[m_dwPacketLength];
	memcpy(m_pbPacketPlaintext, pbPacketHead, PACKET_HEAD_LENGTH);
	if (m_dwPacketBodyLength > 0) {
		memcpy(m_pbPacketPlaintext + PACKET_HEAD_LENGTH, m_pbPacketBody, m_dwPacketBodyLength);
	}

	// 加密封包
	DWORD dwPacketPlaintextLength = m_dwPacketLength;
	DWORD dwPacketCiphertextLength = m_pCrypto->GetCiphertextLength(dwPacketPlaintextLength);
	m_pbPacketCiphertext = new BYTE[dwPacketCiphertextLength];
	m_pCrypto->Encrypt(m_pbPacketPlaintext, dwPacketPlaintextLength, m_pbPacketCiphertext);

	// 封包长度更新为加密后的密文长度，此时m_dwPacketBodyLength包体长度用不到，就不更新了。
	m_dwPacketLength = dwPacketCiphertextLength;
}
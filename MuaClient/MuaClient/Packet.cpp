#include "pch.h"
#include "Packet.h"
#include "SocketClient.h"


// 统一声明：
// 本类中的封包包括封包包头和封包包体
// 封包包头的定义详见PACKET_HEAD结构体
// 并不包括开头的4字节长度哦，这个是由HP-Socket负责的




CPacket::CPacket() {

	m_dwPacketLength = 0;					// 整个封包的长度(包括包头和包体，但不包括封包中表示长度的4个字节)
	m_PacketHead = PACKET_HEAD();			// 包头
	m_pbPacketBody = nullptr;				// 包体

	m_dwPacketBodyLength = 0;				// 包体长度

	m_pbPacketPlaintext = nullptr;			// 封包明文数据（照例不包括开头表示长度的4字节）
	m_pbPacketCiphertext = nullptr;			// 封包密文数据（照例不包括开头表示长度的4字节）

	m_dwPacketPlaintextLength = 0;
	m_dwPacketCiphertextLength = 0;
}


CPacket::CPacket(CSocketClient* pSocketClient) {
	m_dwConnId = 0;
	m_pSocketClient = pSocketClient;
	m_pCrypto = &(m_pSocketClient->m_Crypto);

	m_dwPacketLength = 0;
	m_PacketHead = PACKET_HEAD();
	m_pbPacketBody = nullptr;

	m_dwPacketBodyLength = 0;

	m_pbPacketPlaintext = nullptr;
	m_pbPacketCiphertext = nullptr;

	m_dwPacketPlaintextLength = 0;
	m_dwPacketCiphertextLength = 0;
}


CPacket::~CPacket() {
	if (m_pbPacketBody) {
		delete[] m_pbPacketBody;
		m_pbPacketBody = nullptr;
	}								// 0xdddddddd多半是有指针悬空的锅。

	if (m_pbPacketPlaintext) {
		delete[] m_pbPacketPlaintext;
		m_pbPacketPlaintext = nullptr;
	}

	if (m_pbPacketCiphertext) {
		delete[] m_pbPacketCiphertext;
		m_pbPacketCiphertext = nullptr;
	}
}


// 拷贝构造，用于接收到的数据解析成封包后，传到其他线程里。
// 反正只有封包加密完或者解密完后，才能用这个拷贝构造，因为m_pCrypto没有深拷贝。
// 当然深拷贝也没用，m_pCrypto里面的加解密IV一直变。
CPacket::CPacket(const CPacket& Packet) {
	m_dwConnId = Packet.m_dwConnId;

	m_pSocketClient = Packet.m_pSocketClient;

	m_pCrypto = nullptr;
	// 因为拷贝封包主要是用于传到其他线程里，主要是传包体，所以用不到这项，这里就拷贝了。
	// 而且深拷贝也没用，m_pCrypto里面的加解密IV一直变，必须接收到封包的第一时间就解密，这样才能和Server端同步。

	m_dwPacketLength = Packet.m_dwPacketLength;

	m_PacketHead.wCommandId = Packet.m_PacketHead.wCommandId;
	m_PacketHead.dwCheckSum = Packet.m_PacketHead.dwCheckSum;
	m_PacketHead.bySplitNum = Packet.m_PacketHead.bySplitNum;

	if (Packet.m_dwPacketBodyLength == 0) {
		m_pbPacketBody = new BYTE[1];
		m_pbPacketBody[0] = 0;
	}
	else {
		m_pbPacketBody = new BYTE[Packet.m_dwPacketBodyLength];
		memcpy(m_pbPacketBody, Packet.m_pbPacketBody, Packet.m_dwPacketBodyLength);
	}
	
	m_dwPacketBodyLength = Packet.m_dwPacketBodyLength;

	m_pbPacketPlaintext = new BYTE[Packet.m_dwPacketPlaintextLength];
	memcpy(m_pbPacketPlaintext, Packet.m_pbPacketPlaintext, Packet.m_dwPacketPlaintextLength);

	m_pbPacketCiphertext = new BYTE[Packet.m_dwPacketCiphertextLength];
	memcpy(m_pbPacketCiphertext, Packet.m_pbPacketCiphertext, Packet.m_dwPacketCiphertextLength);		

	m_dwPacketPlaintextLength = Packet.m_dwPacketPlaintextLength;
	m_dwPacketCiphertextLength = Packet.m_dwPacketCiphertextLength;
}


// 解析封包，不可与PacketCombine在同一个Packet对象中同时使用
VOID CPacket::PacketParse(PBYTE pbData, DWORD dwPacketLength) {
	m_dwPacketLength = dwPacketLength;
	m_dwPacketCiphertextLength = m_dwPacketLength;
	m_dwPacketBodyLength = m_dwPacketLength - PACKET_HEAD_LENGTH;

	// 复制封包（密文）
	m_pbPacketCiphertext = new BYTE[m_dwPacketCiphertextLength];
	memcpy(m_pbPacketCiphertext, pbData, dwPacketLength);

	// 解密封包。接收到的封包是密文，解密出来的明文长度一定比密文短，
	// 所以这里直接用密文的长度dwPacketLength就足够了
	m_pbPacketPlaintext = new BYTE[m_dwPacketCiphertextLength];
	m_dwPacketPlaintextLength = m_pCrypto->Decrypt(m_pbPacketCiphertext, m_dwPacketCiphertextLength, m_pbPacketPlaintext);

	// 封包长度更新为解密后的明文封包的长度
	m_dwPacketLength = m_dwPacketPlaintextLength;
	m_dwPacketBodyLength = m_dwPacketLength - PACKET_HEAD_LENGTH;

	// 解析包头
	m_PacketHead = PACKET_HEAD((PBYTE)m_pbPacketPlaintext);

	// 拷贝包体
	if (m_dwPacketBodyLength == 0) {
		m_pbPacketBody = new BYTE[1];
		m_pbPacketBody[0] = 0;
	}
	else {
		m_pbPacketBody = new BYTE[m_dwPacketBodyLength];
		memcpy(m_pbPacketBody, m_pbPacketPlaintext + PACKET_HEAD_LENGTH, m_dwPacketBodyLength);
	}
}


// 组装封包，不可与PacketParse在同一个Packet对象中同时使用
VOID CPacket::PacketCombine(COMMAND_ID wCommandId, PBYTE pbPacketBody, DWORD dwPacketBodyLength) {
	m_dwPacketLength = PACKET_HEAD_LENGTH + dwPacketBodyLength;
	m_dwPacketPlaintextLength = m_dwPacketLength;
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
	m_dwPacketPlaintextLength = m_dwPacketLength;
	m_dwPacketCiphertextLength = m_pCrypto->GetCiphertextLength(m_dwPacketPlaintextLength);
	m_pbPacketCiphertext = new BYTE[m_dwPacketCiphertextLength];
	m_pCrypto->Encrypt(m_pbPacketPlaintext, m_dwPacketPlaintextLength, m_pbPacketCiphertext);

	// 封包长度更新为加密后的密文长度，此时m_dwPacketBodyLength包体长度用不到，就不更新了。
	m_dwPacketLength = m_dwPacketCiphertextLength;
}